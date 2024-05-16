/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "proto/room/room.pb.h"
#include "room/room/room.hpp"
#include "rsplib/util/random.hpp"

namespace rsp {
namespace room {

class room_manager {
 public:
  static room_manager& instance() {
    std::call_once(room_manager::s_flag, []() {
      room_manager::s_instance.reset(new room_manager());
    });
    return *room_manager::s_instance;
  }

  std::shared_ptr<room> create_room(const std::string& uid) {
    RoomId room_id = rsp::libs::util::rng(10000, ULONG_MAX);
    auto created = std::make_shared<room>(room_id, uid);
    std::lock_guard<std::mutex> l(m_);
    // TODO change
    rooms_[room_id] = created;
    return created;
  }

  std::shared_ptr<room> find_room(RoomId room_id) {
    std::lock_guard<std::mutex> l(m_);
    auto room = rooms_.find(room_id);
    if (rooms_.end() == room) {
      return nullptr;
    }
    return room->second;
  }

 private:
  room_manager() {}
  static std::once_flag s_flag;
  static std::unique_ptr<room_manager> s_instance;

  std::mutex m_;
  std::map<RoomId, std::shared_ptr<room>> rooms_;
};

}  // namespace room
}  // namespace rsp
