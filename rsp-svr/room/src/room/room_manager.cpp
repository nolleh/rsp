/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#include "room/room/room_manager.hpp"

namespace rsp {
namespace room {

std::once_flag room_manager::s_flag;
std::unique_ptr<room_manager> room_manager::s_instance;

}  // namespace room
}  // namespace rsp
