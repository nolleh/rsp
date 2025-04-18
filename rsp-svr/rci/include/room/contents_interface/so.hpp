/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <iostream>

#include "room/contents_interface/room_api_interface.hpp"
#include "room/contents_interface/room_message_interface.hpp"
namespace rsp {
namespace room {}
}  // namespace rsp

class so_interface {
 public:
  virtual ~so_interface() {}
  virtual void on_load() = 0;
  virtual void on_unload() = 0;
  virtual rsp::room::room_message_interface* create_room(
      rsp::room::room_api_interface*) = 0;
  virtual void destroy_room() = 0;
};

using so_creator_t = so_interface* (*)();

// }  // namespace room
// }  // namespace rsp
