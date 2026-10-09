/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "room/room/room_manager.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include "room/contents_interface/room_message_interface.hpp"
#include "room/intranet/user_channel.hpp"
#include "rsplib/message/queued_requests.hpp"

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
  // Call only after draining the Room strand.
  static RoutingId member_route(const room& instance, const Uid& uid) {
    return instance.users_.at(uid).route;
  }

  static void post(room_manager& manager, std::function<void()> work) {
    manager.strands_.front().post(std::move(work));
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

TEST(RoomCancellation, QueuedJoinHasNoMembershipOrContentSideEffects) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "route"}, &strand,
      std::make_unique<recording_contents>(&events));
  libs::message::queued_requests requests;
  const libs::message::queued_requests::key key{"route", 1};
  auto ticket = requests.admit(key);
  bool joined = true;
  instance->join_room("guest", "route", [&](bool success) { joined = success; },
                      [ticket] { return ticket->try_start(); });
  requests.cancel(key);
  io_context.run();
  EXPECT_FALSE(joined);
  EXPECT_EQ((std::vector<Uid>{"owner"}), instance->users());
  EXPECT_TRUE(events.empty());
}

TEST(RoomCancellation, QueuedLeaveHasNoMembershipOrContentSideEffects) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "route"}, &strand,
      std::make_unique<recording_contents>(&events));
  libs::message::queued_requests requests;
  const libs::message::queued_requests::key key{"route", 1};
  auto ticket = requests.admit(key);
  bool left = true;
  bool empty = false;
  instance->leave_room("owner", [&](bool success) { left = success; },
                       [&] { empty = true; },
                       [ticket] { return ticket->try_start(); });
  requests.cancel(key);
  io_context.run();
  EXPECT_FALSE(left);
  EXPECT_FALSE(empty);
  EXPECT_EQ((std::vector<Uid>{"owner"}), instance->users());
  EXPECT_TRUE(events.empty());
}

TEST(RoomCancellation, CancellationAfterStartAllowsNormalCompletion) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "route"}, &strand,
      std::make_unique<recording_contents>(&events));
  libs::message::queued_requests requests;
  const libs::message::queued_requests::key key{"route", 1};
  auto ticket = requests.admit(key);
  bool joined = false;
  instance->join_room(
      "guest", "route", [&](bool success) { joined = success; },
      [&] {
        const bool started = ticket->try_start();
        requests.cancel(key);
        return started;
      });
  io_context.run();
  EXPECT_TRUE(joined);
  EXPECT_EQ((std::vector<std::string>{"entered"}), events);
  EXPECT_EQ(2U, instance->users().size());
}

TEST(RoomCancellation, ProtocolCancelsQueuedJoinAndLeaveWithoutReplies) {
  std::vector<std::string> events;
  auto manager = room_manager_test_peer::create_manager();
  auto instance = room_manager_test_peer::create_room(
      *manager, "owner", 12345, std::make_unique<recording_contents>(&events));
  auto context = std::make_shared<zmq::context_t>(1);
  user_channel channel("inproc://queued-cancellation", context, *manager);
  channel.start_async();
  libs::broker::dealer_channel peer(
      "inproc://queued-cancellation", "user-route", context);
  std::promise<void> pong;
  auto pong_result = pong.get_future();
  std::atomic<int> replies{0};
  std::atomic<bool> received_pong{false};
  peer.start([&](libs::message::raw_buffer buffer) {
    auto message = libs::message::serializer::destruct_buffer(buffer);
    if (message.type == MessageType::kPong) {
      if (!received_pong.exchange(true)) pong.set_value();
    } else {
      ++replies;
    }
  });

  auto blocked = std::make_shared<std::promise<void>>();
  auto blocked_result = blocked->get_future();
  std::promise<void> release;
  auto released = release.get_future().share();
  room_manager_test_peer::post(*manager, [blocked, released] {
    blocked->set_value();
    released.wait();
  });
  const auto blocked_status = blocked_result.wait_for(std::chrono::seconds(2));

  User2RoomReqJoinRoom join;
  join.set_request_id(1);
  join.set_uid("guest");
  join.set_room_id(12345);
  peer.send(libs::message::serializer::serialize(
      MessageType::kUser2RoomReqJoinRoom, join));
  User2RoomReqLeaveRoom leave;
  leave.set_request_id(2);
  leave.set_uid("owner");
  peer.send(libs::message::serializer::serialize(
      MessageType::kUser2RoomReqLeaveRoom, leave));
  for (uint64_t id : {1, 2}) {
    User2RoomCancelRequest cancel;
    cancel.set_request_id(id);
    peer.send(libs::message::serializer::serialize(
        MessageType::kUser2RoomCancelRequest, cancel));
  }
  peer.send(libs::message::serializer::serialize(MessageType::kPing, Ping{}));
  // Pong on the ordered route proves both cancellations have been dispatched.
  const auto pong_status = pong_result.wait_for(std::chrono::seconds(2));
  release.set_value();
  auto drained = std::make_shared<std::promise<void>>();
  auto drain_result = drained->get_future();
  room_manager_test_peer::post(*manager, [drained] { drained->set_value(); });
  const auto drain_status = drain_result.wait_for(std::chrono::seconds(2));
  peer.stop();
  channel.stop();

  EXPECT_EQ(std::future_status::ready, blocked_status);
  EXPECT_EQ(std::future_status::ready, pong_status);
  EXPECT_EQ(std::future_status::ready, drain_status);
  EXPECT_EQ(0, replies.load());
  EXPECT_EQ(instance, manager->find_room("owner"));
  EXPECT_EQ(nullptr, manager->find_room("guest"));
  EXPECT_EQ((std::vector<Uid>{"owner"}), instance->users());
  EXPECT_TRUE(events.empty());
}

TEST(RoomCancellation, ProtocolKeepsCompletedJoinAfterLateCancellation) {
  std::vector<std::string> events;
  auto manager = room_manager_test_peer::create_manager();
  auto instance = room_manager_test_peer::create_room(
      *manager, "owner", 12345, std::make_unique<recording_contents>(&events));
  auto context = std::make_shared<zmq::context_t>(1);
  user_channel channel("inproc://completed-cancellation", context, *manager);
  channel.start_async();
  libs::broker::dealer_channel peer(
      "inproc://completed-cancellation", "user-route", context);
  std::promise<bool> joined;
  auto join_result = joined.get_future();
  std::promise<void> pong;
  auto pong_result = pong.get_future();
  std::atomic<bool> got_join{false};
  std::atomic<bool> got_pong{false};
  peer.start([&](libs::message::raw_buffer buffer) {
    auto message = libs::message::serializer::destruct_buffer(buffer);
    if (message.type == MessageType::kUser2RoomResJoinRoom &&
        !got_join.exchange(true)) {
      User2RoomResJoinRoom response;
      libs::message::serializer::deserialize(message.payload, &response);
      joined.set_value(response.success());
    } else if (message.type == MessageType::kPong &&
               !got_pong.exchange(true)) {
      pong.set_value();
    }
  });
  User2RoomReqJoinRoom join;
  join.set_request_id(1);
  join.set_uid("guest");
  join.set_room_id(12345);
  peer.send(libs::message::serializer::serialize(
      MessageType::kUser2RoomReqJoinRoom, join));
  const auto join_status = join_result.wait_for(std::chrono::seconds(2));
  User2RoomCancelRequest cancel;
  cancel.set_request_id(1);
  peer.send(libs::message::serializer::serialize(
      MessageType::kUser2RoomCancelRequest, cancel));
  peer.send(libs::message::serializer::serialize(MessageType::kPing, Ping{}));
  const auto pong_status = pong_result.wait_for(std::chrono::seconds(2));
  auto drained = std::make_shared<std::promise<void>>();
  auto drain_result = drained->get_future();
  room_manager_test_peer::post(*manager, [drained] { drained->set_value(); });
  const auto drain_status = drain_result.wait_for(std::chrono::seconds(2));
  peer.stop();
  channel.stop();

  EXPECT_EQ(std::future_status::ready, join_status);
  if (join_status == std::future_status::ready) EXPECT_TRUE(join_result.get());
  EXPECT_EQ(std::future_status::ready, pong_status);
  EXPECT_EQ(std::future_status::ready, drain_status);
  EXPECT_EQ(instance, manager->find_room("guest"));
  EXPECT_EQ(2U, instance->users().size());
  EXPECT_EQ((std::vector<std::string>{"entered"}), events);
}

TEST(RoomReconnection, RejoinRefreshesOwnerAndGuestRoutesWithoutDuplicateEnter) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "old-owner-route"}, &strand,
      std::make_unique<recording_contents>(&events));
  int successes = 0;
  auto completed = [&](bool joined) { if (joined) ++successes; };

  instance->join_room("guest", "old-guest-route", completed);
  instance->join_room("owner", "new-owner-route", completed);
  instance->join_room("guest", "new-guest-route", completed);
  instance->join_room("guest", "new-guest-route", completed);
  io_context.run();

  EXPECT_EQ(4, successes);
  EXPECT_EQ(2U, instance->users().size());
  EXPECT_EQ("new-owner-route",
            room_manager_test_peer::member_route(*instance, "owner"));
  EXPECT_EQ("new-guest-route",
            room_manager_test_peer::member_route(*instance, "guest"));
  EXPECT_EQ((std::vector<std::string>{"entered"}), events);
}

TEST(RoomReconnection, CancelledRejoinPreservesPreviousRoute) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "old-route"}, &strand,
      std::make_unique<recording_contents>(&events));
  libs::message::queued_requests requests;
  const libs::message::queued_requests::key key{"new-route", 1};
  auto ticket = requests.admit(key);
  bool joined = true;

  instance->join_room(
      "owner", "new-route", [&](bool success) { joined = success; },
      [ticket] { return ticket->try_start(); });
  requests.cancel(key);
  io_context.run();

  EXPECT_FALSE(joined);
  EXPECT_EQ("old-route",
            room_manager_test_peer::member_route(*instance, "owner"));
  EXPECT_EQ((std::vector<Uid>{"owner"}), instance->users());
  EXPECT_TRUE(events.empty());
}

TEST(RoomReconnection, EarlierConnectionLeaveMayRemoveRejoinedMember) {
  boost::asio::io_context io_context;
  boost::asio::io_context::strand strand(io_context);
  std::vector<std::string> events;
  auto instance = std::make_shared<room>(
      12345, user{"owner", "owner-route"}, &strand,
      std::make_unique<recording_contents>(&events));
  bool left = false;
  bool empty = false;

  instance->join_room("guest", "old-route",
                      [](bool joined) { EXPECT_TRUE(joined); });
  instance->join_room("guest", "new-route",
                      [](bool joined) { EXPECT_TRUE(joined); });
  // Leave remains UID-based: an earlier connection's request is not rejected.
  instance->leave_room("guest", [&](bool success) { left = success; },
                       [&] { empty = true; });
  io_context.run();

  EXPECT_TRUE(left);
  EXPECT_FALSE(empty);
  EXPECT_EQ((std::vector<Uid>{"owner"}), instance->users());
  EXPECT_EQ((std::vector<std::string>{"entered", "exited"}), events);
}

}  // namespace room
}  // namespace rsp
