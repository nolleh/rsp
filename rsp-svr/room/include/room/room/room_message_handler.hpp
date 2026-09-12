/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"
#include "room/room/room_manager.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;

// class intranet;

class room_message_handler {
 public:
  room_message_handler()
      : logger_(lg::logger()), room_manager_(room_manager::instance()) {}

  Pong handle(const Ping& ping) {
    logger_.trace() << "ping " << lg::L_endl;
    Pong pong;
    return pong;
  }

  User2RoomResCreateRoom handle(const User2RoomReqCreateRoom& create_room) {
    logger_.trace() << "create_room: " << create_room.DebugString()
                    << lg::L_endl;

    // TODO(@nolleh) do not initialize here. let's make glob. pubsub
    register_user_server(create_room);

    auto room =
        room_manager_.create_room(create_room.uid(), create_room.addr());

    room->create_room();

    User2RoomResCreateRoom res_create_room;
    res_create_room.set_success(true);
    res_create_room.set_request_id(create_room.request_id());
    res_create_room.set_room_id(room->room_id());
    logger_.debug() << "created_room:" << room->room_id() << lg::L_endl;
    return res_create_room;
  }

  User2RoomResJoinRoom handle(const User2RoomReqJoinRoom& join_room) {
    logger_.trace() << "join_room: " << join_room.DebugString() << lg::L_endl;
    register_user_server(join_room);

    auto room = room_manager_.join_room(join_room.uid(), join_room.room_id(),
                                        join_room.addr());
    if (!room) {
      User2RoomResJoinRoom res_join_room;
      res_join_room.set_request_id(join_room.request_id());
      res_join_room.set_room_id(join_room.room_id());
      res_join_room.set_success(false);
      return res_join_room;
    }

    room->join_room(join_room.uid(), join_room.addr());
    User2RoomResJoinRoom res_join_room;
    res_join_room.set_request_id(join_room.request_id());
    res_join_room.set_room_id(join_room.room_id());
    res_join_room.set_success(true);
    return res_join_room;
  }

  void handle(const User2RoomFwdRoom& fwd_room) {
    logger_.trace() << "fwd_room: " << fwd_room.DebugString() << lg::L_endl;
    auto room = room_manager_.find_room(fwd_room.uid());
    if (!room) {
      logger_.warn() << "unable to forward message from unknown user: "
                     << fwd_room.uid() << lg::L_endl;
      return;
    }

    logger_.trace() << "fwd_room, run room handler" << lg::L_endl;
    room->on_recv_message(fwd_room.uid(), fwd_room.message());
    logger_.trace() << "finished run room handler" << lg::L_endl;
  }

  User2RoomResLeaveRoom handle(const User2RoomReqLeaveRoom& leave_room) {
    logger_.trace() << "leave_room: " << leave_room.DebugString() << lg::L_endl;

    const auto room = room_manager_.leave_room(leave_room.uid());

    User2RoomResLeaveRoom res_leave_room;
    res_leave_room.set_request_id(leave_room.request_id());
    if (!room) {
      res_leave_room.set_success(false);
      return res_leave_room;
    }

    room->leave_room(leave_room.uid());
    res_leave_room.set_success(true);
    return res_leave_room;
  }

 private:
  void register_user_server(const User2RoomReqCreateRoom& msg);
  void register_user_server(const User2RoomReqJoinRoom& msg);

  lg::s_logger& logger_;
  room_manager& room_manager_;
};
}  // namespace room
}  // namespace rsp
