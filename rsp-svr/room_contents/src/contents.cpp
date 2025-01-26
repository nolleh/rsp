/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <iostream>

#include "room_contents/so.hpp"

extern "C" {
so_interface* create() {
  std::cout << "create so_interface in room_contents....." << std::endl;
  return new so{};
}
}

void so::on_load() { std::cout << "[room_contents] on_load" << std::endl; }
void so::on_unload() { std::cout << "[room_contents] on_unload" << std::endl; }

void so::on_create_room() {
  std::cout << "[room_contents] on_create_room" << std::endl;
}

void so::on_destroy_room() {
  std::cout << "[room_contents] on_destroy_room" << std::endl;
}
