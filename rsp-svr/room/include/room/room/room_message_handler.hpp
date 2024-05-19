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

class intranet;

class room_message_handler {
 public:
  explicit room_message_handler(intranet* intranet)
      : intranet_(intranet),
        logger_(lg::logger()),
        room_manager_(room_manager::instance()) {}

  Pong handle(const Ping& ping) {
    logger_.trace() << "ping " << lg::L_endl;
    Pong pong;
    return pong;
  }

  ResCreateRoom handle(const ReqCreateRoom& create_room) {
    logger_.trace() << "create_room: uid(" << create_room.uid() << ")"
                    << lg::L_endl;
    auto room = room_manager_.create_room(create_room.uid());
    ResCreateRoom res_create_room;
    res_create_room.set_success(true);
    res_create_room.set_request_id(create_room.request_id());
    res_create_room.set_room_id(room->room_id());
    logger_.debug() << "created_room:" << room->room_id() << lg::L_endl;
    return res_create_room;
  }

  ResJoinRoom handle(const ReqJoinRoom& join_room) {
    logger_.trace() << "join_room: room_id(" << join_room.room_id() << "), uid("
                    << join_room.uid() << lg::L_endl;
    auto room = room_manager_.find_room(join_room.room_id());
    room->join_room(join_room.uid());
    ResJoinRoom res_join_room;
    res_join_room.set_request_id(join_room.request_id());
    res_join_room.set_room_id(join_room.room_id());
    return res_join_room;
  }

 private:
  lg::s_logger& logger_;
  room_manager& room_manager_;
  intranet* intranet_;
};
}  // namespace room
}  // namespace rsp
