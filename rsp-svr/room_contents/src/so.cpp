/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "room_contents/so.hpp"
#include "room_contents/contents.hpp"

extern "C" {
so_interface* create() {
  std::cout << "create so_interface in room_contents....." << std::endl;
  return new so{};
}
}

void so::on_load() { std::cout << "[room_contents] on_load" << std::endl; }
void so::on_unload() { std::cout << "[room_contents] on_unload" << std::endl; }

rsp::room::room_message_interface* so::create_room() {
  std::cout << "[room_contents] on_create_room" << std::endl;
  return new rsp::room::contents::contents{};
}

void so::destroy_room() {
  std::cout << "[room_contents] on_destroy_room" << std::endl;
}
