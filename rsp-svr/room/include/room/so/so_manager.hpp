
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <memory>

#include "room/contents_interface/so.hpp"
#include "room/so/so_factory.hpp"

namespace rsp {
namespace room {

class so_manager {
 public:
  static so_manager& instance() {
    static so_manager manager;
    return manager;
  }

  ~so_manager() { unload(); }

  void load() {
    so_interface_ = factory_.create();
    so_interface_->on_load();
  }
  void unload() {
    if (so_interface_) so_interface_->on_unload();
  }

  room_message_interface* contents_interface(room_api_interface* api) {
    return so_interface_->create_room(api);
  }

 private:
  rsp::room::so_factory factory_;
  std::unique_ptr<so_interface> so_interface_;
};

}  // namespace room
}  // namespace rsp
