/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <algorithm>
// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h> // NOLINT
#include <string>

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

TEST(Interpreter, QueuedMessage) {
  namespace message = rsp::libs::message;
  message::conn_interpreter interpreter;

  std::string m = "hello, world";
  std::string m2 = "hello, world2";

  auto type = MessageType::kReqFwdClient;

  ReqFwdClient fwdClient;
  fwdClient.set_message("hello, world");
  auto buffer = message::serializer::serialize(type, fwdClient);
  auto bufSize = buffer.size();

  ReqFwdClient fwdClient2;
  fwdClient2.set_message("hello, world2");
  auto buffer2 = message::serializer::serialize(type, fwdClient2);
  auto buf2Size = buffer2.size();

  buffer.insert(buffer.end(), buffer2.begin(), buffer2.end());
  EXPECT_EQ(buffer.size(), bufSize + buf2Size);

  std::array<char, 128> stream;
  std::copy(buffer.begin(), buffer.end(), stream.begin());
  message::message_dispatcher::instance().register_handler(
      MessageType::kReqFwdClient,
      [&](message::buffer_ptr buffer, rsp::libs::link::link* link) {
        ReqFwdClient fwd;
        auto deserialized = message::serializer::deserialize(*buffer, &fwd);
        EXPECT_EQ(m, fwd.message());
      });
  interpreter.handle_buffer(stream, buffer.size());

  message::message_dispatcher::instance().register_handler(
      MessageType::kReqFwdClient,
      [&](message::buffer_ptr buffer, rsp::libs::link::link* link) {
        ReqFwdClient fwd;
        auto deserialized = message::serializer::deserialize(*buffer, &fwd);
        EXPECT_EQ(m2, fwd.message());
      });
  interpreter.handle_buffer(std::array<char, 128>{}, 0);
}
