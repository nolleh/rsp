/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/

#include "room/intranet/room_receiver.hpp"

namespace rsp {
namespace room {

template <>
void room_receiver::handle(const User2RoomReqFwdRoom& msg) {
  logger_.trace() << "on_recv: " << typeid(msg).name() << lg::L_endl;
  message_handler_.handle(msg);
}
}  // namespace room
}  // namespace rsp
