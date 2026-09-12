/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <functional>
#include <utility>

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

  Pong handle(const Ping& ping, const RoutingId&) {
    logger_.trace() << "ping " << lg::L_endl;
    Pong pong;
    return pong;
  }

  User2RoomResCreateRoom handle(const User2RoomReqCreateRoom& create_room,
                                const RoutingId& source) {
    logger_.trace() << "create_room: " << create_room.DebugString()
                    << lg::L_endl;

    auto room = room_manager_.create_room(create_room.uid(), source);

    room->create_room();

    User2RoomResCreateRoom res_create_room;
    res_create_room.set_success(true);
    res_create_room.set_request_id(create_room.request_id());
    res_create_room.set_room_id(room->room_id());
    logger_.debug() << "created_room:" << room->room_id() << lg::L_endl;
    return res_create_room;
  }

  void handle(
      const User2RoomReqJoinRoom& join_room, const RoutingId& source,
      std::function<void(User2RoomResJoinRoom)> complete) {
    logger_.trace() << "join_room: " << join_room.DebugString() << lg::L_endl;

    auto room = room_manager_.join_room(join_room.uid(), join_room.room_id());
    User2RoomResJoinRoom res_join_room;
    res_join_room.set_request_id(join_room.request_id());
    res_join_room.set_room_id(join_room.room_id());
    if (!room) {
      res_join_room.set_success(false);
      complete(std::move(res_join_room));
      return;
    }

    res_join_room.set_success(true);
    room->join_room(join_room.uid(), source,
                    [complete = std::move(complete),
                     response = std::move(res_join_room)]() mutable {
                      complete(std::move(response));
                    });
  }

  void handle(const User2RoomFwdRoom& fwd_room, const RoutingId&) {
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

  void handle(
      const User2RoomReqLeaveRoom& leave_room, const RoutingId&,
      std::function<void(User2RoomResLeaveRoom)> complete) {
    logger_.trace() << "leave_room: " << leave_room.DebugString() << lg::L_endl;

    const auto room = room_manager_.leave_room(leave_room.uid());

    User2RoomResLeaveRoom res_leave_room;
    res_leave_room.set_request_id(leave_room.request_id());
    if (!room) {
      res_leave_room.set_success(false);
      complete(std::move(res_leave_room));
      return;
    }

    res_leave_room.set_success(true);
    room->leave_room(leave_room.uid(),
                     [complete = std::move(complete),
                      response = std::move(res_leave_room)]() mutable {
                       complete(std::move(response));
                     });
  }

 private:
  lg::s_logger& logger_;
  room_manager& room_manager_;
};
}  // namespace room
}  // namespace rsp
