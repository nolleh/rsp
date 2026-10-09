/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#include "rspcli/state/state_login.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <optional>

namespace rsp::cli::state {
namespace {

raw_buffer creation_payload(bool success, RoomId room_id) {
  ResCreateRoom response;
  response.set_request_id(1);
  response.set_success(success);
  response.set_room_id(room_id);
  raw_buffer payload(response.ByteSizeLong());
  response.SerializeToArray(payload.data(), static_cast<int>(payload.size()));
  return payload;
}

TEST(StateLogin, StaysInLobbyAfterFailedCreation) {
  context context;
  context.uid = "nolleh";
  context.room_id = 0;
  auto state = state_login::create(&context, [](MessageType, raw_buffer) {});
  const auto next = state->on_message(
      MessageType::kResCreateRoom,
      std::make_shared<raw_buffer>(creation_payload(false, 0)));
  EXPECT_EQ(next, std::nullopt);
  EXPECT_EQ(context.room_id, 0U);
}

TEST(StateLogin, EntersRoomAfterSuccessfulCreation) {
  context context;
  context.uid = "nolleh";
  context.room_id = 0;
  auto state = state_login::create(&context, [](MessageType, raw_buffer) {});
  const auto next = state->on_message(
      MessageType::kResCreateRoom,
      std::make_shared<raw_buffer>(creation_payload(true, 42)));
  ASSERT_TRUE(next.has_value());
  EXPECT_EQ(*next, State::kInRoom);
  EXPECT_EQ(context.room_id, 42U);
}
}  // namespace
}  // namespace rsp::cli::state
