/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <iostream>
#include <string>

#include "room/contents_interface/room_message_interface.hpp"
#include "room/contents_interface/types.hpp"

namespace rsp {
namespace room {
namespace contents {

class contents : rsp::room::room_message_interface {
  /**
   * room is created
   * this is first interface that always sent right after created the object
   * */
  void on_created_room(const RoomId room_id) {
    std::cout << "[contents]" << "on_created_room" << std::endl;
  }

  /**
   * user entered to room
   * */
  void on_user_enter(const Uid uid) {
    std::cout << "[contents]" << "on_user_enter" << std::endl;
  }

  /**
   * about to destroyed room.
   * this is last interface that sent before destroy the object
   * */
  void on_destroy_room() {
    std::cout << "[contents]" << "on_destroy_room" << std::endl;
  }

  /**
   * in room, message was received
   * */
  void on_recv_message(Uid from, const std::string msg) {
    std::cout << "[contents]" << "on_recv_message" << std::endl;
  }

  /**
   * interface that called when user in room was kicked out.
   * e.g., by using api::kick_out_user, after process was done,
   * this callback will be popped up
   * */
  void on_kicked_out_user(Uid uid, KickOutReason reason) {
    std::cout << "[contents]" << "on_kicked_out_user" << std::endl;
  }
};

}  // namespace contents
}  // namespace room
}  // namespace rsp
