/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <algorithm>
// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>  // NOLINT
#include <stdexcept>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_client.pb.h"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/serializer.hpp"

TEST(gTest, SimpleTest) {
  int a = 0;
  int b = 0;

  EXPECT_EQ(a, b);

  a = 1;

  EXPECT_NE(a, b);
}

TEST(Message, Serialize) {
  namespace message = rsp::libs::message;

  auto type = MessageType::kReqLogin;
  ReqLogin login;
  login.set_uid("nolleh");

  auto buffer = message::serializer::serialize(type, login);
  auto destructured = message::serializer::destruct_buffer(buffer);

  ReqLogin login2;
  auto result = message::serializer::deserialize(destructured.payload, &login2);

  EXPECT_TRUE(result);
  EXPECT_EQ(login.uid(), login2.uid());
}

TEST(Message, PayloadDoesNotIncludeFollowingMessage) {
  namespace message = rsp::libs::message;

  FwdClient first;
  first.set_message("first");
  FwdClient second;
  second.set_message("second");

  auto first_buffer =
      message::serializer::serialize(MessageType::kFwdClient, first);
  auto second_buffer =
      message::serializer::serialize(MessageType::kFwdClient, second);
  first_buffer.insert(first_buffer.end(), second_buffer.begin(),
                      second_buffer.end());

  const auto meta = message::serializer::destruct_buffer(first_buffer);
  ASSERT_NE(meta.size, 0u);

  FwdClient decoded;
  ASSERT_TRUE(message::serializer::deserialize(meta.payload, &decoded));
  EXPECT_EQ(decoded.message(), first.message());
  EXPECT_EQ(meta.payload_size, first.ByteSizeLong());
}

TEST(Message, AcceptsPayloadAtMaximumSize) {
  namespace message = rsp::libs::message;

  message::raw_buffer buffer;
  const auto content_length = message::serializer::kMaxPayloadSize;
  message::mset(&buffer, content_length);
  message::mset(&buffer, static_cast<int>(MessageType::kFwdClient));
  buffer.resize(message::serializer::kHeaderSize + content_length);

  const auto meta = message::serializer::destruct_buffer(buffer);

  EXPECT_EQ(meta.status, message::parse_status::kComplete);
  EXPECT_EQ(meta.payload_size, content_length);
}

TEST(Message, RejectsPayloadLargerThanMaximumSize) {
  namespace message = rsp::libs::message;

  message::raw_buffer buffer;
  const auto content_length = message::serializer::kMaxPayloadSize + 1;
  message::mset(&buffer, content_length);

  const auto meta = message::serializer::destruct_buffer(buffer);

  EXPECT_EQ(meta.status, message::parse_status::kInvalid);
}

TEST(Message, WaitsForIncompletePayloadWithinLimit) {
  namespace message = rsp::libs::message;

  message::raw_buffer buffer;
  const auto content_length = message::serializer::kMaxPayloadSize;
  message::mset(&buffer, content_length);

  const auto meta = message::serializer::destruct_buffer(buffer);

  EXPECT_EQ(meta.status, message::parse_status::kIncomplete);
}

TEST(Message, RejectsOversizedPayloadOnSerialize) {
  namespace message = rsp::libs::message;

  FwdClient oversized;
  oversized.set_message(
      std::string(message::serializer::kMaxPayloadSize + 1, 'x'));

  EXPECT_THROW(
      message::serializer::serialize(MessageType::kFwdClient, oversized),
      std::length_error);
}

TEST(Interpreter, QueuedMessage) {
  namespace message = rsp::libs::message;
  message::conn_interpreter interpreter;

  std::string m = "hello, world";
  std::string m2 = "hello, world2";

  auto type = MessageType::kFwdClient;

  FwdClient fwdClient;
  fwdClient.set_message("hello, world");
  auto buffer = message::serializer::serialize(type, fwdClient);
  auto bufSize = buffer.size();

  FwdClient fwdClient2;
  fwdClient2.set_message("hello, world2");
  auto buffer2 = message::serializer::serialize(type, fwdClient2);
  auto buf2Size = buffer2.size();

  buffer.insert(buffer.end(), buffer2.begin(), buffer2.end());
  EXPECT_EQ(buffer.size(), bufSize + buf2Size);

  std::array<char, 128> stream;
  std::copy(buffer.begin(), buffer.end(), stream.begin());
  std::vector<std::string> received;
  message::message_dispatcher::instance().register_handler(
      MessageType::kFwdClient,
      [&received](message::buffer_ptr buffer, rsp::libs::link::link* link) {
        FwdClient fwd;
        auto deserialized = message::serializer::deserialize(*buffer, &fwd);
        ASSERT_TRUE(deserialized);
        received.push_back(fwd.message());
      });
  interpreter.handle_buffer(stream, buffer.size());

  EXPECT_EQ(received, (std::vector<std::string>{m, m2}));
}

TEST(Interpreter, DispatchesAllCompleteMessagesFromOneRead) {
  namespace message = rsp::libs::message;

  message::conn_interpreter interpreter;
  std::vector<std::string> received;

  FwdClient first;
  first.set_message("first");
  FwdClient second;
  second.set_message("second");

  auto first_buffer =
      message::serializer::serialize(MessageType::kFwdClient, first);
  auto second_buffer =
      message::serializer::serialize(MessageType::kFwdClient, second);
  first_buffer.insert(first_buffer.end(), second_buffer.begin(),
                      second_buffer.end());

  ASSERT_LE(first_buffer.size(), 128u);
  std::array<char, 128> input{};
  std::copy(first_buffer.begin(), first_buffer.end(), input.begin());

  message::message_dispatcher::instance().register_handler(
      MessageType::kFwdClient,
      [&received](message::buffer_ptr buffer, rsp::libs::link::link* link) {
        FwdClient decoded;
        ASSERT_TRUE(message::serializer::deserialize(*buffer, &decoded));
        received.push_back(decoded.message());
      });

  interpreter.handle_buffer(input, first_buffer.size());

  EXPECT_EQ(received, (std::vector<std::string>{"first", "second"}));
}

TEST(Interpreter, RejectsOversizedFrame) {
  namespace message = rsp::libs::message;

  const auto content_length = message::serializer::kMaxPayloadSize + 1;
  message::raw_buffer header;
  message::mset(&header, content_length);

  std::array<char, 128> input{};
  std::copy(header.begin(), header.end(), input.begin());

  message::conn_interpreter interpreter;
  EXPECT_FALSE(interpreter.handle_buffer(input, header.size()));
}
