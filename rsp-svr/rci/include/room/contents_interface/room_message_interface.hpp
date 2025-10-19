/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <string>
#include <vector>

#include "room/contents_interface/kickout_reason.hpp"
#include "room/contents_interface/types.hpp"

namespace rsp {
namespace room {

class room_message_interface {
 public:
  /**
   * room is created
   * this is first interface that always sent right after created the object
   * */
  virtual void on_create_room(const RoomId room_id) = 0;

  /**
   * user entered to room
   * */
  virtual void on_user_enter(const Uid uid) = 0;

  /**
   * about to destroyed room.
   * this is last interface that sent before destroy the object
   * */
  virtual void on_destroy_room() = 0;

  /**
   * in room, message was received
   * */
  virtual void on_recv_message(const Uid& from, const std::string& msg) = 0;

  /**
   * interface that called when user in room was kicked out.
   * e.g., by using api::kick_out_user, after process was done,
   * this callback will be popped up
   * */
  virtual void on_kicked_out_user(const Uid& uid,
                                  const KickoutReason& reason) = 0;
};

}  // namespace room
}  // namespace rsp
