/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

namespace rsp {
namespace room {

class so_interface {
 public:
  virtual ~so_interface() {}
  virtual void on_load() {}
  virtual void on_unload() {}
  virtual void on_create_room() {}
  virtual void on_destroy_room() {}
};

}  // namespace room
}  // namespace rsp
