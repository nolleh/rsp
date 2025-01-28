/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <string>

#include "room/contents_interface/room_message_interface.hpp"
#include "room/contents_interface/types.hpp"

namespace rsp {
namespace room {
namespace contents {

class contents : public rsp::room::room_message_interface {
 public:
  /**
   * room is created
   * this is first interface that always sent right after created the object
   * */
  void on_create_room(const RoomId room_id);

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

}  // namespace contents
}  // namespace room
}  // namespace rsp
