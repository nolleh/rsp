/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "proto/room/room.pb.h"
#include "room/room/room_api_interface.hpp"
#include "room/room/room_message_interface.hpp"
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
  explicit user(const Uid& uid, const Address& addr) : uid(uid), addr(addr) {}

  const Uid uid;
  const Address addr;
};

class room : public room_api_inteface,
             public room_message_interface,
             public std::enable_shared_from_this<room> {
 public:
  room(RoomId room_id, user user, ba::io_context::strand* strand)
      : room_id_(room_id),
        owner_(user),
        users_{{user.uid, user}},
        strand_(strand),
        logger_(lg::logger()) {}

  ~room() {
    strand_->post(std::bind(&room::on_destroy_room, shared_from_this()));
  }

  void create_room() {
    strand_->post(
        std::bind(&room::on_created_room, shared_from_this(), room_id_));
  }

  void join_room(const Uid& uid, const Address& addr) {
    strand_->post(
        std::bind(&room::on_user_enter, shared_from_this(), uid, addr));
  }

  RoomId room_id() { return room_id_; }

  bool send_to_user(const std::vector<Uid> uids, const std::string& msg) {
    return send_to_user(SenderType::kContent, nullptr, uids, msg);
  }

  bool send_to_user(const user& sender, const std::vector<Uid> uids,
                    const std::string& msg) {
    return send_to_user(SenderType::kUser, std::make_shared<user>(sender), uids,
                        msg);
  }

  void send_to_all_user(const std::string& msg) {
    return send_to_all_user(SenderType::kContent, nullptr, msg);
  }

  void send_to_all_user(const user& sender, const std::string& msg) {
    return send_to_all_user(SenderType::kUser, std::make_shared<user>(sender),
                            msg);
  }

  void kick_out_user(Uid uid) {}

  void on_created_room(const RoomId room_id) {}

  void on_user_enter(const Uid& uid, const Address& addr) {
    users_.insert({uid, user(uid, addr)});
    send_to_all_user("uid:(" + uid + ") has entered room");
  }

  void on_destroy_room() {}

  void on_recv_message(Uid from, const std::string& msg) {
    logger_.debug() << "message from user(" << from << "): message: " << msg
                    << lg::L_endl;
    // echo
    auto user = users_.find(from);
    if (users_.end() == user) {
      logger_.debug() << "unable to find out sender(" << from
                      << "), message: " << msg << lg::L_endl;

      strand_->post(
          boost::bind(&room::send_to_all_user, shared_from_this(), msg));
      return;
    }

    strand_->post(boost::bind(&room::send_to_all_user, shared_from_this(),
                              user->second, msg));
  }

  void on_kicked_out_user(Uid uid, KickOutReason reason) {}

 private:
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
    strand_->dispatch(std::bind(&room::send_to_user_impl, shared_from_this(),
                                sender_type, sender,
                                std::make_shared<std::vector<user>>(users),
                                rsp::libs::buffer::make_buffer_ptr(msg)));
  }

  void send_to_user_impl(const SenderType& sender_type,
                         const std::shared_ptr<user> sender,
                         const std::shared_ptr<std::vector<user>> users,
                         const lm::buffer_ptr buffer);

  RoomId room_id_;
  user owner_;
  std::map<Uid, user> users_;
  ba::io_context::strand* strand_;
  lg::s_logger& logger_;
};

}  // namespace room
}  // namespace rsp
