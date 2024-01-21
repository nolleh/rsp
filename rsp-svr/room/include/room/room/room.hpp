/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <vector>

#include "boost/asio.hpp"
#include "rsplib/thread/thread_pool.hpp"

namespace rsp {
namespace room {

namespace ba = boost::asio;
using RoomId = long;

class user {
 public:
  user(std::string uid) : uid_(uid){};

 private:
  std::string uid_;
};

class room {
 public:
  room(RoomId room_id, std::string uid)
      : room_id_(room_id), users_({user(uid)}), workers_(3) {}
  void create_room() {}
  void join_room() {}

  RoomId room_id() {
    return room_id_;
  }

 private:
  RoomId room_id_;
  std::vector<user> users_;
  rsp::libs::thread::thread_pool workers_;
  ba::io_context::strand* strand_;
};
}  // namespace room
}  // namespace rsp
