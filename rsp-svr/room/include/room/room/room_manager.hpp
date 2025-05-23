/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "proto/room/room.pb.h"
#include "room/room/room.hpp"
#include "rsplib/thread/thread_pool.hpp"
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

  ~room_manager() { workers_.stop(); }

  std::shared_ptr<room> create_room(const std::string& uid,
                                    const std::string& addr) {
    // temporarily for test convenient
    RoomId room_id = rsp::libs::util::rng(10000, 100000);
    // auto room_id = (--rooms_.end())->first + 1;
    auto created =
        std::make_shared<room>(room_id, user{uid, addr},
                               &strands_.at(rooms_.size() % strands_.size()));

    created->create_room();

    std::lock_guard<std::mutex> l(m_);
    // TODO(@nolleh) change
    rooms_[room_id] = created;
    user_rooms_[uid] = room_id;
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

  std::shared_ptr<room> join_room(const std::string& uid, const RoomId room_id,
                                  const std::string& addr) {
    auto room = find_room(room_id);
    if (!room) {
      return nullptr;
    }

    rooms_[room_id] = room;
    user_rooms_[uid] = room_id;
    room->join_room(uid, addr);
    return room;
  }

  std::shared_ptr<room> find_room(Uid uid) {
    RoomId room_id = 0;
    {
      std::lock_guard<std::mutex> l(m_);
      auto room_iter = user_rooms_.find(uid);
      if (user_rooms_.end() == room_iter) {
        return nullptr;
      }
      room_id = room_iter->second;
    }
    return find_room(room_id);
  }

 private:
  room_manager()
      : workers_(30),
        strands_{workers_.size() / 10,
                 ba::io_context::strand(*workers_.io_context())} {
    workers_.start();
  }

  static std::once_flag s_flag;
  static std::unique_ptr<room_manager> s_instance;

  std::mutex m_;
  std::map<RoomId, std::shared_ptr<room>> rooms_;
  std::map<Uid, RoomId> user_rooms_;

  rsp::libs::thread_pool workers_;
  std::vector<ba::io_context::strand> strands_;
};

}  // namespace room
}  // namespace rsp
