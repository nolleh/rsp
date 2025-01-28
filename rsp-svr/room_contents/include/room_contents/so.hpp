/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "room/contents_interface/so.hpp"
#include "room_contents/contents.hpp"

class so : public so_interface {
 public:
  so() {}
  void on_load() override;
  void on_unload() override;
  rsp::room::room_message_interface* on_create_room() override;
  void on_destroy_room() override;
};
