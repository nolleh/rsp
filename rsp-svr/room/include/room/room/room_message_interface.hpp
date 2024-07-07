/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <string>
#include <vector>

#include "room/types.hpp"
#include "room/room/kick_out_reason.hpp"

namespace rsp {
namespace room {

class room_message_interface {
  /**
   * room is created
   * this is first interface that always sent right after created the object
   * */
  void on_created_room(const RoomId room_id);

  /**
   * user entered to room
   * */
  void on_user_enter(const Uid uid);

  /**
   * about to destroyed room.
   * this is last interface that sent before destroy the object
   * */
  void on_destroy_room();

  /**
   * in room, message was received
   * */
  void on_recv_message(Uid from, const std::string msg);

  /**
   * interface that called when user in room was kicked out.
   * e.g., by using api::kick_out_user, after process was done,
   * this callback will be popped up
   * */
  void on_kicked_out_user(Uid uid, KickOutReason reason);
};

}  // namespace room
}  // namespace rsp
