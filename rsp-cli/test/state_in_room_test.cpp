/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/state/state_in_room.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <vector>

#include "proto/user/to_room.pb.h"

namespace rsp::cli::state {
namespace {

raw_buffer payload_of(const google::protobuf::Message& message) {
  raw_buffer payload(message.ByteSizeLong());
  message.SerializeToArray(payload.data(), static_cast<int>(payload.size()));
  return payload;
}

context room_context() {
  context result;
  result.uid = "nolleh";
  result.room_id = 42;
  return result;
}

TEST(StateInRoom, SendsLeaveRequestForLeaveCommand) {
  context context = room_context();
  std::vector<MessageType> sent_types;
  auto state = state_in_room::create(
      &context, [&sent_types](MessageType type, raw_buffer) {
        sent_types.push_back(type);
      });

  EXPECT_EQ(state->on_command("3"), std::nullopt);
  ASSERT_EQ(sent_types.size(), 1U);
  EXPECT_EQ(sent_types.front(), MessageType::kReqLeaveRoom);
}

TEST(StateInRoom, SendsChatAsOneWayForwardMessage) {
  context context = room_context();
  std::vector<MessageType> sent_types;
  auto state = state_in_room::create(
      &context, [&sent_types](MessageType type, raw_buffer) {
        sent_types.push_back(type);
      });

  EXPECT_EQ(state->on_command("2"), std::nullopt);
  EXPECT_EQ(state->on_command("hello"), std::nullopt);

  ASSERT_EQ(sent_types.size(), 1U);
  EXPECT_EQ(sent_types.front(), MessageType::kFwdRoom);
}

TEST(StateInRoom, TransitionsToLoggedInAfterSuccessfulLeave) {
  context context = room_context();
  auto state = state_in_room::create(&context, [](MessageType, raw_buffer) {});
  ResLeaveRoom response;
  response.set_success(true);

  const auto next =
      state->on_message(MessageType::kResLeaveRoom,
                        std::make_shared<raw_buffer>(payload_of(response)));

  ASSERT_TRUE(next.has_value());
  EXPECT_EQ(*next, State::kLoggedIn);
  EXPECT_EQ(context.room_id, 0U);
}

TEST(StateInRoom, StaysInRoomAfterFailedLeave) {
  context context = room_context();
  auto state = state_in_room::create(&context, [](MessageType, raw_buffer) {});
  ResLeaveRoom response;
  response.set_success(false);

  const auto next =
      state->on_message(MessageType::kResLeaveRoom,
                        std::make_shared<raw_buffer>(payload_of(response)));

  EXPECT_EQ(next, std::nullopt);
  EXPECT_EQ(context.room_id, 42U);
}

}  // namespace
}  // namespace rsp::cli::state
