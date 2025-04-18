/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "room/contents_interface/so.hpp"
#include "room_contents/contents.hpp"

class so : public so_interface {
 public:
  so() {}
  void on_load() override;
  void on_unload() override;
  rsp::room::room_message_interface* create_room(
      rsp::room::room_api_interface*) override;
  void destroy_room() override;
};
