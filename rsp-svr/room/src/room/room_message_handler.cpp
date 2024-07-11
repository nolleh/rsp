/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#include "room/room/room_message_handler.hpp"

#include "proto/room/room.pb.h"
#include "room/intranet/intranet.hpp"

namespace rsp {
namespace room {

void room_message_handler::register_user_server(
    const User2RoomReqCreateRoom& msg) {
  // TODO(@nolleh) this!
  // intranet::instance();
  // intranet::instance().user().register_server(msg.addr());
}
void room_message_handler::register_user_server(
    const User2RoomReqJoinRoom& msg) {
  intranet::instance().user().register_server(msg.addr());
}

}  // namespace room
}  // namespace rsp
