/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <algorithm>
#include <exception>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "proto/room/room.pb.h"
#include "room/contents_interface/room_api_interface.hpp"
#include "room/contents_interface/room_message_interface.hpp"
#include "room/so/so_manager.hpp"
#include "room/types.hpp"
#include "rsplib/buffer/buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace ba = boost::asio;
namespace lm = rsp::libs::message;

struct user {
  explicit user(const Uid& uid, const RoutingId& route)
      : uid(uid), route(route) {}

  const Uid uid;
  const RoutingId route;
};

class room : public room_api_interface,
             public std::enable_shared_from_this<room> {
 public:
  room(RoomId room_id, user owner, ba::io_context::strand* strand)
      : room(room_id, std::move(owner), strand, nullptr) {}

  room(RoomId room_id, user owner, ba::io_context::strand* strand,
       std::unique_ptr<room_message_interface> contents)
      : room_id_(room_id),
        owner_(std::move(owner)),
        users_{{owner_.uid, owner_}},
        strand_(strand),
        logger_(lg::logger()),
        contents_(std::move(contents)) {}

  ~room() = default;

  void create_room() {
    contents_ = std::unique_ptr<room_message_interface>(
        rsp::room::so_manager::instance().contents_interface(this));
    strand_->post(
        std::bind(&room::on_create_room, shared_from_this(), room_id_));
  }

  void join_room(const Uid& uid, const RoutingId& route,
                 std::function<void(bool)> before_notify) {
    strand_->post([self = shared_from_this(), uid, route,
                   before_notify = std::move(before_notify)] {
      if (self->lifecycle_ != lifecycle::kOpen) {
        before_notify(false);
        return;
      }

      self->users_.insert({uid, user(uid, route)});
      before_notify(true);
      self->contents_->on_user_enter(uid);
    });
  }

  void leave_room(const Uid& uid, std::function<void(bool)> before_notify,
                  std::function<void()> on_empty) {
    strand_->post([self = shared_from_this(), uid,
                   before_notify = std::move(before_notify),
                   on_empty = std::move(on_empty)] {
      if (self->lifecycle_ != lifecycle::kOpen ||
          self->users_.erase(uid) == 0) {
        before_notify(false);
        return;
      }

      const bool empty = self->users_.empty();
      before_notify(true);
      self->contents_->on_user_exit(uid);
      if (empty) on_empty();
    });
  }

  std::future<void> close() {
    auto completed = std::make_shared<std::promise<void>>();
    auto result = completed->get_future();
    strand_->dispatch([self = shared_from_this(), completed] {
      try {
        self->close_impl();
        completed->set_value();
      } catch (...) {
        completed->set_exception(std::current_exception());
      }
    });
    return result;
  }

  RoomId room_id() const { return room_id_; }

  std::vector<Uid> users() override {
    std::vector<Uid> users;
    std::transform(users_.cbegin(), users_.cend(), std::back_inserter(users),
                   [](const auto& pair) { return pair.second.uid; });
    return users;
  }

  bool send_to_user(const std::vector<Uid>& uids,
                    const std::string& msg) override {
    return send_to_user(SenderType::kContent, nullptr, uids, msg);
  }

  bool send_to_user(const user& sender, const std::vector<Uid>& uids,
                    const std::string& msg) {
    return send_to_user(SenderType::kUser, std::make_shared<user>(sender), uids,
                        msg);
  }

  void send_to_all_user(const std::string& msg) {
    return send_to_all_user(SenderType::kContent, nullptr, msg);
  }

  void send_to_all_user(const user& sender, const std::string& msg) {
    logger_.trace() << "send_to_all user pop" << lg::L_endl;
    return send_to_all_user(SenderType::kUser, std::make_shared<user>(sender),
                            msg);
  }

  void kick_out_user(const Uid& uid, const KickoutReason& reason) override {
    logger_.debug() << "kick out user(" << uid << ")";

    strand_->post(
        std::bind(&room::kick_out_user_impl, shared_from_this(), uid, reason));
  }

  void on_create_room(const RoomId room_id) {
    contents_->on_create_room(room_id);
  }

  void on_recv_message(Uid from, const std::string& msg) {
    contents_->on_recv_message(from, msg);
  }

  void on_kicked_out_user(const Uid& uid, const KickoutReason& reason) {}

 private:
  enum class lifecycle { kOpen, kClosing, kClosed };

  void close_impl() {
    if (lifecycle_ != lifecycle::kOpen) return;

    lifecycle_ = lifecycle::kClosing;
    if (contents_) {
      try {
        contents_->on_destroy_room();
      } catch (const std::exception& exception) {
        logger_.error() << "room contents failed to close: "
                        << exception.what() << lg::L_endl;
      } catch (...) {
        logger_.error() << "room contents failed to close" << lg::L_endl;
      }
      contents_.reset();
    }
    lifecycle_ = lifecycle::kClosed;
  }

  bool send_to_user(const SenderType sender_type,
                    const std::shared_ptr<user>& sender,
                    const std::vector<Uid>& uids, const std::string& msg) {
    std::vector<user> users;
    std::for_each(uids.cbegin(), uids.cend(), [this, &users](const auto& uid) {
      auto iter = users_.find(uid);
      if (users_.end() != iter) {
        users.push_back(iter->second);
      }
    });

    if (uids.size() != users.size()) {
      logger_.debug() << "some of user unable to find.." << lg::L_endl;
      return false;
    }

    strand_->post(std::bind(&room::send_to_user_impl, shared_from_this(),
                            sender_type, sender,
                            std::make_shared<std::vector<user>>(users),
                            rsp::libs::buffer::make_buffer_ptr(msg)));

    return true;
  }

  void send_to_all_user(const SenderType& sender_type,
                        const std::shared_ptr<user>& sender,
                        const std::string& msg) {
    std::vector<user> users;
    std::transform(users_.cbegin(), users_.cend(), std::back_inserter(users),
                   [](const auto& pair) { return pair.second; });
    logger_.debug() << "send_to_all_user, # of users: " << users.size()
                    << lg::L_endl;
    strand_->post(std::bind(&room::send_to_user_impl, shared_from_this(),
                            sender_type, sender,
                            std::make_shared<std::vector<user>>(users),
                            rsp::libs::buffer::make_buffer_ptr(msg)));
  }

  void send_to_user_impl(const SenderType& sender_type,
                         const std::shared_ptr<user> sender,
                         const std::shared_ptr<std::vector<user>> users,
                         const lm::buffer_ptr buffer);

  void kick_out_user_impl(const Uid& uid, const KickoutReason& reason);

  RoomId room_id_;
  user owner_;
  std::map<Uid, user> users_;
  ba::io_context::strand* strand_;
  lg::s_logger& logger_;
  std::unique_ptr<room_message_interface> contents_;
  lifecycle lifecycle_{lifecycle::kOpen};
};

}  // namespace room
}  // namespace rsp
