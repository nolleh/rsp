/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "room/room/constant.hpp"
#include "room/room/room_api_interface.hpp"
#include "room/room/room_message_interface.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/thread/thread_pool.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace ba = boost::asio;

class user {
 public:
  explicit user(Uid uid) : uid_(uid) {}

 private:
  Uid uid_;
  // TODO(@nolleh) user server information
};

class room : public room_api_inteface, public room_message_interface {
 public:
  room(RoomId room_id, Uid uid)
      : room_id_(room_id),
        users_({user(uid)}),
        workers_(3),
        logger_(lg::logger()) {}

  ~room() { on_destroy_room(); }

  void create_room() { on_created_room(room_id_); }

  void join_room(Uid uid) { on_user_enter(uid); }

  RoomId room_id() { return room_id_; }

  void send_to_user(const std::vector<Uid> uids, const std::string& msg) {
    // TODO(@nolleh) by using user server information, send the msg to intranet
  }

  void send_to_all_user(const std::string& msg) {}

  void kick_out_user(Uid uid) {}

  void on_created_room(const RoomId room_id) {}

  void on_user_enter(Uid uid) {}

  void on_destroy_room() {}

  void on_recv_message(Uid from, const std::string msg) {
    logger_.debug() << "message from user(" << from << "): message: " << msg
                    << lg::L_endl;

    // echo
    send_to_all_user(msg);
  }

  void on_kicked_out_user(Uid uid, KickOutReason reason) {}

 private:
  RoomId room_id_;
  std::vector<user> users_;
  rsp::libs::thread::thread_pool workers_;
  ba::io_context::strand* strand_;
  lg::s_logger& logger_;
};

}  // namespace room
}  // namespace rsp
