/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/broker/broker.hpp"
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

TEST(Broker, CreateAnyCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kAnyCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  // auto address = pub->get_addr();
  // auto prefix = address.substr(0, 6);
  // EXPECT_EQ(prefix, "A@test");
}

TEST(Broker, CreateUniCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kUniCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
}

TEST(Broker, CreateBroadCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kBroadCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
}

TEST(Broker, CreateAnyCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kAnyCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
  // auto address = pub->get_addr();
  // auto prefix = address.substr(0, 6);
  // EXPECT_EQ(prefix, "A@test");
}

TEST(Broker, CreateUniCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kUniCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
}

TEST(Broker, CreateBroadCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kBroadCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
}

TEST(Broker, PubSub) {
  namespace br = rsp::libs::broker;
  auto pub = br::broker::s_create_publisher(CastType::kBroadCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);

  auto sub =
      br::broker::s_create_subscriber(CastType::kBroadCast, "test", 1, "topic");
  EXPECT_TRUE(sub != nullptr);

  pub->start();
  sub->start();
  std::string msg = "test";
  rsp::libs::message::raw_buffer buffer{msg.begin(), msg.end()};
  pub->send("topic", buffer);

  auto recv_msg = sub->recv("topic").get();
  EXPECT_EQ(msg.size(), recv_msg.size());
  auto str_recv_msg = std::string{recv_msg.data(), recv_msg.size()};
  EXPECT_EQ(msg, str_recv_msg);
}
