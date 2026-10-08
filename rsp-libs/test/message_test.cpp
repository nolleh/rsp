/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <algorithm>
#include <cstdint>
#include <limits>
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
  message::mset_be(&buffer, static_cast<uint64_t>(content_length));
  message::mset_be(&buffer, static_cast<uint32_t>(MessageType::kFwdClient));
  buffer.resize(message::serializer::kHeaderSize + content_length);

  const auto meta = message::serializer::destruct_buffer(buffer);

  EXPECT_EQ(meta.status, message::parse_status::kComplete);
  EXPECT_EQ(meta.payload_size, content_length);
}

TEST(Message, RejectsPayloadLargerThanMaximumSize) {
  namespace message = rsp::libs::message;

  message::raw_buffer buffer;
  const auto content_length = message::serializer::kMaxPayloadSize + 1;
  message::mset_be(&buffer, static_cast<uint64_t>(content_length));

  const auto meta = message::serializer::destruct_buffer(buffer);

  EXPECT_EQ(meta.status, message::parse_status::kInvalid);
}

TEST(Message, WaitsForIncompletePayloadWithinLimit) {
  namespace message = rsp::libs::message;

  message::raw_buffer buffer;
  const auto content_length = message::serializer::kMaxPayloadSize;
  message::mset_be(&buffer, static_cast<uint64_t>(content_length));

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
  message::mset_be(&header, static_cast<uint64_t>(content_length));

  std::array<char, 128> input{};
  std::copy(header.begin(), header.end(), input.begin());

  message::conn_interpreter interpreter;
  EXPECT_FALSE(interpreter.handle_buffer(input, header.size()));
}

TEST(Message, SerializesFixedWidthNetworkOrderHeader) {
  namespace message = rsp::libs::message;
  static_assert(message::serializer::kHeaderSize == 12);

  ReqLogin login;
  login.set_uid("a");
  const auto buffer =
      message::serializer::serialize(MessageType::kReqLogin, login);
  const message::raw_buffer expected{
      0, 0, 0, 0, 0, 0, 0, 3,  // uint64_t payload length
      0, 0, 0, 2,              // uint32_t message type
      0x12, 1, 'a'};          // protobuf payload
  EXPECT_EQ(buffer, expected);
}

TEST(Message, ParsesFixedWidthNetworkOrderHeader) {
  namespace message = rsp::libs::message;
  const message::raw_buffer buffer{
      0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, 0x12, 1, 'a'};
  const auto meta = message::serializer::destruct_buffer(buffer);
  ASSERT_EQ(meta.status, message::parse_status::kComplete);
  EXPECT_EQ(meta.size, 15u);
  EXPECT_EQ(meta.payload_size, 3u);
  EXPECT_EQ(meta.type, MessageType::kReqLogin);
  EXPECT_EQ(meta.payload, (message::raw_buffer{0x12, 1, 'a'}));
}

TEST(Message, WaitsForEveryPartialHeader) {
  namespace message = rsp::libs::message;
  const message::raw_buffer header{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2};
  for (size_t length = 0; length < header.size(); ++length) {
    const message::raw_buffer partial(header.begin(), header.begin() + length);
    EXPECT_EQ(message::serializer::destruct_buffer(partial).status,
              message::parse_status::kIncomplete);
  }
  EXPECT_EQ(message::serializer::destruct_buffer(header).status,
            message::parse_status::kComplete);
}

TEST(Message, RejectsWireLengthBeforeNarrowingToSizeT) {
  namespace message = rsp::libs::message;
  message::raw_buffer buffer;
  message::mset_be(&buffer, uint64_t{1} << 32);
  EXPECT_EQ(message::serializer::destruct_buffer(buffer).status,
            message::parse_status::kInvalid);

  buffer.clear();
  message::mset_be(&buffer, std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(message::serializer::destruct_buffer(buffer).status,
            message::parse_status::kInvalid);
}

TEST(Message, NetworkOrderHelpersPreserveEveryByte) {
  namespace message = rsp::libs::message;
  message::raw_buffer buffer;
  message::mset_be(&buffer, uint64_t{0x0123456789abcdef});
  message::mset_be(&buffer, uint32_t{0x89abcdef});
  const unsigned char expected[]{
      0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
      0x89, 0xab, 0xcd, 0xef};
  ASSERT_EQ(buffer.size(), sizeof(expected));
  for (size_t index = 0; index < buffer.size(); ++index) {
    EXPECT_EQ(static_cast<unsigned char>(buffer[index]), expected[index]);
  }

  uint64_t length = 0;
  uint32_t type = 0;
  ASSERT_TRUE(message::mget_be(buffer, &length, 0));
  ASSERT_TRUE(message::mget_be(buffer, &type, 8));
  EXPECT_EQ(length, uint64_t{0x0123456789abcdef});
  EXPECT_EQ(type, uint32_t{0x89abcdef});
  EXPECT_FALSE(message::mget_be(buffer, &type, 9));
  EXPECT_FALSE(message::mget_be(
      buffer, &type, std::numeric_limits<size_t>::max()));
}
