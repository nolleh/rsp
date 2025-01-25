/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "room/contents_interface/so.hpp"
#include "room_contents/contents.hpp"

class so_interface : public rsp::room::so_interface {
 public:
  so_interface() {}
  void on_load() override {}
  void on_unload() override {}
  void on_create_room() override {}
  void on_destroy_room() override {}
};

extern "C" {
rsp::room::so_interface* create() { return new so_interface{}; }
}
