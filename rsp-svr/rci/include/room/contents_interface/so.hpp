/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <iostream>
namespace rsp {
namespace room {

class room_message_interface;

}
}  // namespace rsp

class so_interface {
 public:
  virtual ~so_interface() {}
  virtual void on_load() = 0;
  virtual void on_unload() = 0;
  virtual rsp::room::room_message_interface* on_create_room() = 0;
  virtual void on_destroy_room() = 0;
};

using so_creator_t = so_interface* (*)();

// }  // namespace room
// }  // namespace rsp
