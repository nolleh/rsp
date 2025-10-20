/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/

#include "room_contents/contents.hpp"

#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace rsp {
namespace room {
namespace contents {

void contents::on_create_room(const RoomId room_id) {
  std::cout << "[contents]" << "on_create_enter" << std::endl;
}

void contents::on_user_enter(const Uid uid) {
  std::cout << "[contents]" << "on_user_enter" << std::endl;
  api_->send_to_user(api_->users(), std::format("({0}) has entered", uid));
}

void contents::on_destroy_room() {
  std::cout << "[contents]" << "on_destroy_room" << std::endl;
}

void contents::on_recv_message(const Uid& from, const std::string& msg) {
  std::cout << "[contents]" << "on_recv_message" << std::endl;
  // TODO(@nolleh) content message protocol

  std::vector<std::string> tokens;
  std::string token;
  char delimiter = ':';
  std::istringstream token_stream{msg};

  while (std::getline(token_stream, token, delimiter)) {
    tokens.push_back(token);
  }

  if (tokens.size() > 1) {
    // TODO(@nolleh) make feature to check this user is room owner
    if ("kickout" == tokens[0]) {
      api_->kick_out_user(tokens[1], KickoutReason::kRoomOwner);
      return;
    }
  }
  // echo
  api_->send_to_user(api_->users(), std::format("({0}):{1}", from, msg));
}

void contents::on_kicked_out_user(const Uid& uid, const KickoutReason& reason) {
  std::cout << "[contents]" << "on_kicked_out_user" << std::endl;
}

}  // namespace contents
}  // namespace room
}  // namespace rsp
