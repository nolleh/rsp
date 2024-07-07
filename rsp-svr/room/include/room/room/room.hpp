/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "room/room/room_api_interface.hpp"
#include "room/room/room_message_interface.hpp"
#include "room/types.hpp"
#include "rsplib/buffer/buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/types.hpp"
#include "rsplib/thread/thread_pool.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace ba = boost::asio;
namespace lm = rsp::libs::message;

struct user {
  explicit user(const Uid& uid, const Address& addr) : uid(uid), addr(addr) {}

  Uid uid;
  Address addr;
  // TODO(@nolleh) user server information
};

class room : public room_api_inteface,
             public room_message_interface,
             public std::enable_shared_from_this<room> {
 public:
  room(RoomId room_id, user user)
      : room_id_(room_id),
        users_{{user.uid, user}},
        workers_(3),
        io_context_(workers_.io_context()),
        strand_(*io_context_),
        logger_(lg::logger()) {
  }

  ~room() { on_destroy_room(); }

  void create_room() { on_created_room(room_id_); }

  void join_room(const Uid& uid, const Address& addr) {
    on_user_enter(uid, addr);
  }

  RoomId room_id() { return room_id_; }

  bool send_to_user(const std::vector<Uid> uids, const std::string& msg) {
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

    strand_.post(std::bind(&room::send_to_user_impl, shared_from_this(),
                            std::make_shared<std::vector<user>>(users),
                            rsp::libs::buffer::make_buffer_ptr(msg)));

    return true;
  }

  void send_to_all_user(const std::string& msg) {
    std::vector<user> users;
    std::transform(users_.cbegin(), users_.cend(), std::back_inserter(users),
                   [](const auto& pair) { return pair.second; });
    strand_.dispatch(std::bind(&room::send_to_user_impl, shared_from_this(),
                                std::make_shared<std::vector<user>>(users),
                                rsp::libs::buffer::make_buffer_ptr(msg)));
  }

  void kick_out_user(Uid uid) {}

  void on_created_room(const RoomId room_id) {}

  void on_user_enter(const Uid& uid, const Address& addr) {
    users_.insert({uid, user(uid, addr)});
  }

  void on_destroy_room() {}

  void on_recv_message(Uid from, const std::string msg) {
    logger_.debug() << "message from user(" << from << "): message: " << msg
                    << lg::L_endl;

    // echo
    strand_.post(std::bind(&room::send_to_all_user, shared_from_this(), msg));
    logger_.debug() << "strand post done" << msg
                    << lg::L_endl;
  }

  void on_kicked_out_user(Uid uid, KickOutReason reason) {}

 private:
  void send_to_user_impl(const std::shared_ptr<std::vector<user>> users,
                         const lm::buffer_ptr buffer) {
    // TODO(@nolleh) by using user server information, send the msg to intranet
  }

  RoomId room_id_;
  std::map<Uid, user> users_;
  rsp::libs::thread::thread_pool workers_;
  ba::io_context* io_context_;
  ba::io_context::strand strand_;
  lg::s_logger& logger_;
};

}  // namespace room
}  // namespace rsp
