/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "room/room/room_manager.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include "room/contents_interface/room_message_interface.hpp"

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

  static void remove_mapping(room_manager& manager, const Uid& uid,
                             RoomId room_id) {
    std::lock_guard<std::mutex> lock(manager.m_);
    manager.user_rooms_.erase(uid);
    manager.rooms_.erase(room_id);
  }

  static std::shared_ptr<room> create_room(
      room_manager& manager, const Uid& uid, RoomId room_id,
      std::unique_ptr<room_message_interface> contents) {
    std::lock_guard<std::mutex> lock(manager.m_);
    auto instance = std::make_shared<room>(
        room_id, user{uid, "route"}, &manager.strands_.front(),
        std::move(contents));
    manager.rooms_[room_id] = instance;
    manager.user_rooms_[uid] = room_id;
    return instance;
  }
};

class recording_contents : public room_message_interface {
 public:
  explicit recording_contents(std::vector<std::string>* events)
      : events_(events) {}

  void on_create_room(RoomId) override { events_->push_back("created"); }
  void on_user_enter(Uid) override { events_->push_back("entered"); }
  void on_user_exit(Uid) override { events_->push_back("exited"); }
  void on_destroy_room() override { events_->push_back("destroyed"); }
  void on_recv_message(const Uid&, const std::string&) override {}
  void on_kicked_out_user(const Uid&, const KickoutReason&) override {}

 private:
  std::vector<std::string>* events_;
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
  room_manager_test_peer::remove_mapping(*manager, uid, kRoomId);
}

TEST(RoomManager, LeaveUnknownUserReturnsNull) {
  auto manager = room_manager_test_peer::create_manager();

  EXPECT_EQ(nullptr, manager->leave_room("unknown-leaving-user"));
}

TEST(RoomManager, RemovesRoomAfterLastUserLeaves) {
  auto manager = room_manager_test_peer::create_manager();
  constexpr RoomId kRoomId = 12345;
  const Uid uid = "owner";
  std::vector<std::string> events;
  auto instance = room_manager_test_peer::create_room(
      *manager, uid, kRoomId,
      std::make_unique<recording_contents>(&events));
  auto leaving = manager->leave_room(uid);
  std::promise<void> closed;
  auto close_result = closed.get_future();

  ASSERT_EQ(instance, leaving);
  leaving->leave_room(
      uid, [](bool) {},
      [&manager, &closed, instance] {
        manager->close_room(instance).get();
        closed.set_value();
      });

  ASSERT_EQ(std::future_status::ready,
            close_result.wait_for(std::chrono::seconds(1)));
  EXPECT_EQ(nullptr, manager->find_room(kRoomId));
  EXPECT_EQ((std::vector<std::string>{"exited", "destroyed"}), events);
}

TEST(RoomManager, ClosesRoomsBeforeWorkerShutdown) {
  std::vector<std::string> events;
  auto manager = room_manager_test_peer::create_manager();
  auto instance = room_manager_test_peer::create_room(
      *manager, "owner", 12345,
      std::make_unique<recording_contents>(&events));

  manager.reset();

  EXPECT_EQ((std::vector<std::string>{"destroyed"}), events);
}

TEST(RoomLifecycle, LastUserLeaveClosesRoomOnStrand) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "route"}, &strand,
      std::make_unique<recording_contents>(&events));

  instance->leave_room(
      "owner",
      [&events](bool left) {
        EXPECT_TRUE(left);
        events.push_back("response");
      },
      [&events, instance] {
        events.push_back("empty");
        instance->close();
      });

  io_context.run();

  EXPECT_EQ((std::vector<std::string>{"response", "exited", "empty",
                                      "destroyed"}),
            events);
}

TEST(RoomLifecycle, CloseNotifiesContentsOnlyOnce) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "route"}, &strand,
      std::make_unique<recording_contents>(&events));

  auto first = instance->close();
  auto second = instance->close();
  io_context.run();
  first.get();
  second.get();

  EXPECT_EQ((std::vector<std::string>{"destroyed"}), events);
}

}  // namespace room
}  // namespace rsp
