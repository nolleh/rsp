/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/

#include "room_contents/contents.hpp"

#include <iostream>
#include <string>

namespace rsp {
namespace room {
namespace contents {

void contents::on_create_room(const RoomId room_id) {
  std::cout << "[contents]" << "on_create_enter" << std::endl;
}

void contents::on_user_enter(const Uid uid) {
  std::cout << "[contents]" << "on_user_enter" << std::endl;
}

void contents::on_destroy_room() {
  std::cout << "[contents]" << "on_destroy_room" << std::endl;
}

void contents::on_recv_message(Uid from, const std::string msg) {
  std::cout << "[contents]" << "on_recv_message" << std::endl;
}

void on_kicked_out_user(Uid uid, KickOutReason reason) {
  std::cout << "[contents]" << "on_kicked_out_user" << std::endl;
}

}  // namespace contents
}  // namespace room
}  // namespace rsp
