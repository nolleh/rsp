/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#include <stdexcept>
#include <string>

#include "room/intranet/intranet.hpp"
#include "room/room/room_manager.hpp"
#include "room/so/so_manager.hpp"

int main(int argc, char** argv) {
  using namespace rsp::room;
  if (argc != 2) return 2;
  const std::string mode = argv[1];
  if (mode == "unloaded") {
    // A dependency may be constructed without its on_load ever running.
    intranet::instance();
    return 0;
  }

  if (mode == "manager-first") {
    room_manager::instance();
    so_manager::instance().load();
    intranet::instance();
  } else if (mode == "channel-first") {
    intranet::instance();
    so_manager::instance().load();
  } else {
    so_manager::instance().load();
    intranet::instance();
  }

  auto& manager = room_manager::instance();
  auto first = manager.create_room("first", "first-route");
  first->create_room();
  // Leave a live room owned by the real manager at normal process exit.
  first.reset();

  if (mode == "explicit") {
    intranet::instance().stop();
    manager.shutdown();
    manager.shutdown();
    try {
      manager.create_room("late", "late-route");
      return 3;
    } catch (const std::logic_error&) {
      // Admission must remain closed after an explicit shutdown.
    }
  }
  return 0;  // Exercise static destruction rather than signal termination.
}
