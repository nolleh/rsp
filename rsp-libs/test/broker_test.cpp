/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>

#include "rsplib/broker/broker.hpp"

TEST(Broker, CreateAnyCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kAnyCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  // auto address = pub->get_addr();
  // auto prefix = address.substr(0, 6);
  // EXPECT_EQ(prefix, "A@test");
  pub->stop();
}

TEST(Broker, CreateUniCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kUniCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
  pub->stop();
}

TEST(Broker, CreateBroadCastPublisher) {
  namespace br = rsp::libs::broker;

  auto pub = br::broker::s_create_publisher(CastType::kBroadCast, "test", 1);
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
  pub->stop();
}

TEST(Broker, CreateAnyCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kAnyCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
  // auto address = pub->get_addr();
  // auto prefix = address.substr(0, 6);
  // EXPECT_EQ(prefix, "A@test");
  pub->stop();
}

TEST(Broker, CreateUniCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kUniCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
  //   auto address = pub->get_addr();
  //   auto prefix = address.substr(0, 6);
  //   EXPECT_EQ(prefix, "U@test");
  pub->stop();
}

TEST(Broker, CreateBroadCastSubscriber) {
  namespace br = rsp::libs::broker;

  auto pub =
      br::broker::s_create_subscriber(CastType::kBroadCast, "test", 1, "topic");
  EXPECT_TRUE(pub != nullptr);
  pub->stop();
}

TEST(Broker, PubSub) {
  namespace br = rsp::libs::broker;
  auto pub = br::broker::s_create_publisher(CastType::kBroadCast, "test", 1);
  EXPECT_TRUE(nullptr != pub);

  auto sub =
      br::broker::s_create_subscriber(CastType::kBroadCast, "test", 1, "topic");
  EXPECT_TRUE(nullptr != sub);

  pub->start();
  sub->start();
  std::string msg = "test";
  rsp::libs::message::raw_buffer buffer{msg.begin(), msg.end()};
  pub->send("topic", buffer);

  auto recv_msg = sub->recv("topic").get();
  EXPECT_EQ(msg.size(), recv_msg.size());
  auto str_recv_msg = std::string{recv_msg.data(), recv_msg.size()};
  EXPECT_EQ(msg, str_recv_msg);
  pub->stop();
  sub->stop();
}

TEST(Broker, RepSend) {
  namespace br = rsp::libs::broker;
  auto rep = br::broker::s_create_subscriber(CastType::kUniCast, "service", 1, "topic");
  EXPECT_TRUE(nullptr != rep);
  rep->start();
  // wait few sec for bind finished
  // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::string msg = "hello";
  rsp::libs::message::raw_buffer response{msg.begin(), msg.end()};
  rep->send("topic", response);
  rep->stop();
}

// TEST(Broker, ReqRep) {
//   namespace br = rsp::libs::broker;
//   auto req = br::broker::s_create_publisher(CastType::kUniCast, "test", 1);
//   EXPECT_TRUE(nullptr != req);
//   auto rep = br::broker::s_create_subscriber(CastType::kUniCast, "service", 1, "topic");
//   EXPECT_TRUE(nullptr != rep);
//
//   req->start();
//   rep->start();
//
//   std::string msg = "hello";
//   rsp::libs::message::raw_buffer buffer{msg.begin(), msg.end()};
//
//   req->send("service", buffer);
//   auto recv_msg = rep->recv("service").get();
//   EXPECT_EQ(msg.size(), recv_msg.size());
//   auto str_recv_msg = std::string{recv_msg.data(), recv_msg.size()};
//   EXPECT_EQ(msg, str_recv_msg);
//
//   rsp::libs::message::raw_buffer response{msg.begin(), msg.end()};
//   rep->send("topic", response);
//  
//   // wait to finished
//   req->stop();
//   rep->stop();
// }
