
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <memory>
#include <mutex>

#include "room/contents_interface/so.hpp"
#include "room/so/so_factory.hpp"

namespace rsp {
namespace room {

class so_manager {
 public:
  static so_manager& instance() {
    std::call_once(so_manager::s_flag,
                   []() { so_manager::s_instance.reset(new so_manager()); });
    return *so_manager::s_instance;
  }

  ~so_manager() { unload(); }

  void load() {
    so_interface_ = factory_.create();
    so_interface_->on_load();
  }
  void unload() { so_interface_->on_unload(); }

  room_message_interface* contents_interface() {
    return so_interface_->create_room();
  }

 private:
  static std::once_flag s_flag;
  static std::unique_ptr<so_manager> s_instance;
  rsp::room::so_factory factory_;
  std::unique_ptr<so_interface> so_interface_;
};

}  // namespace room
}  // namespace rsp
