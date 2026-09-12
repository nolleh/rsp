/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "room/room/room_manager.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>

namespace rsp {
namespace room {

class room_manager_test_peer {
 public:
  static std::unique_ptr<room_manager> create_manager() {
    return std::unique_ptr<room_manager>(new room_manager());
  }

  static void add_mapping(room_manager& manager, const Uid& uid, RoomId room_id,
                          const std::shared_ptr<room>& mapped_room) {
    std::lock_guard<std::mutex> lock(manager.m_);
    manager.rooms_[room_id] = mapped_room;
    manager.user_rooms_[uid] = room_id;
  }
};

TEST(RoomManager, LeaveRoomReturnsMappedRoom) {
  auto manager = room_manager_test_peer::create_manager();
  constexpr RoomId kRoomId = 12345;
  const Uid uid = "leaving-user";

  // The alias points at aligned storage but owns only the harmless backing
  // allocation. The room is never dereferenced in this mapping-only test.
  static std::aligned_storage_t<sizeof(room), alignof(room)> room_storage;
  auto backing = std::make_shared<std::byte>();
  auto mapped_room = std::shared_ptr<room>(
      std::move(backing), reinterpret_cast<room*>(&room_storage));

  room_manager_test_peer::add_mapping(*manager, uid, kRoomId, mapped_room);

  EXPECT_EQ(mapped_room, manager->leave_room(uid));
  EXPECT_EQ(nullptr, manager->find_room(uid));
}

TEST(RoomManager, LeaveUnknownUserReturnsNull) {
  auto manager = room_manager_test_peer::create_manager();

  EXPECT_EQ(nullptr, manager->leave_room("unknown-leaving-user"));
}

}  // namespace room
}  // namespace rsp
