/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <future>
#include <utility>
#include <vector>

#include "proto/room/room.pb.h"
#include "room/room/room.hpp"
#include "rsplib/thread/thread_pool.hpp"
#include "rsplib/util/random.hpp"

namespace rsp {
namespace room {

class room_manager_test_peer;

class room_manager {
 public:
  static room_manager& instance() {
    std::call_once(room_manager::s_flag, []() {
      room_manager::s_instance.reset(new room_manager());
    });
    return *room_manager::s_instance;
  }

  ~room_manager() {
    std::vector<std::shared_ptr<room>> rooms;
    {
      std::lock_guard<std::mutex> lock(m_);
      for (auto& [room_id, instance] : rooms_) {
        rooms.push_back(std::move(instance));
      }
      rooms_.clear();
      user_rooms_.clear();
    }

    std::vector<std::future<void>> closed;
    closed.reserve(rooms.size());
    for (const auto& instance : rooms) {
      closed.push_back(instance->close());
    }
    for (auto& result : closed) {
      result.get();
    }

    rooms.clear();
    workers_.stop();
  }

  std::shared_ptr<room> create_room(const std::string& uid,
                                    const RoutingId& route) {
    // temporarily for test convenient
    RoomId room_id = rsp::libs::util::rng(10000, 100000);
    // auto room_id = (--rooms_.end())->first + 1;
    std::lock_guard<std::mutex> l(m_);
    auto created =
        std::make_shared<room>(room_id, user{uid, route},
                               &strands_.at(rooms_.size() % strands_.size()));
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

  bool joined_room(const Uid& uid, const std::shared_ptr<room>& instance) {
    std::lock_guard<std::mutex> l(m_);
    auto found = rooms_.find(instance->room_id());
    if (rooms_.end() == found || found->second != instance) return false;

    user_rooms_[uid] = instance->room_id();
    return true;
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

  std::shared_ptr<room> leave_room(Uid uid) {
    RoomId room_id = 0;
    {
      std::lock_guard<std::mutex> l(m_);
      auto room_iter = user_rooms_.find(uid);
      if (user_rooms_.end() == room_iter) {
        return nullptr;
      }
      room_id = room_iter->second;
      user_rooms_.erase(room_iter);
    }

    return find_room(room_id);
  }

  std::future<void> close_room(const std::shared_ptr<room>& instance) {
    {
      std::lock_guard<std::mutex> lock(m_);
      auto found = rooms_.find(instance->room_id());
      if (rooms_.end() == found || found->second != instance) {
        std::promise<void> completed;
        completed.set_value();
        return completed.get_future();
      }

      rooms_.erase(found);
      for (auto user_room = user_rooms_.begin();
           user_room != user_rooms_.end();) {
        if (user_room->second == instance->room_id()) {
          user_room = user_rooms_.erase(user_room);
        } else {
          ++user_room;
        }
      }
    }

    return instance->close();
  }

 private:
  friend class room_manager_test_peer;

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
