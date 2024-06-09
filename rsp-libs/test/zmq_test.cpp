/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/

// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

TEST(ZMQ_REQREP, ReqSocketSend) {
  zmq::context_t ctx{};
  zmq::socket_t req{ctx, zmq::socket_type::req};
  req.connect("inproc://test");

  std::string msg{"hello, world"};
  zmq::message_t request{msg};

  auto rc = req.send(request, zmq::send_flags::dontwait);
  EXPECT_EQ(msg.size(), rc);
}

TEST(ZMQ_REQREP, RepSocketRecvBuffer) {
  zmq::context_t context;
  zmq::socket_t sender{context, zmq::socket_type::req};
  zmq::socket_t receiver{context, zmq::socket_type::rep};

  receiver.bind("inproc://test");
  sender.connect("inproc://test");

  const char *str = "Hi";
  EXPECT_EQ(2, *sender.send(zmq::buffer(str, 2)));

  char buf[2];
  const auto res = receiver.recv(zmq::buffer(buf));
  EXPECT_TRUE(res);
  EXPECT_TRUE(!res->truncated());
  EXPECT_EQ(2, res->size);
}

TEST(ZMQ_REQREP, RepSocketSendResponseInProc) {
  zmq::context_t context;
  zmq::socket_t sender{context, zmq::socket_type::req};
  zmq::socket_t receiver{context, zmq::socket_type::rep};

  receiver.bind("inproc://test");
  sender.connect("inproc://test");

  const char *str = "Hi";
  EXPECT_EQ(2, *sender.send(zmq::buffer(str, 2)));

  char buf[2];
  const auto res = receiver.recv(zmq::buffer(buf));
  EXPECT_TRUE(res);
  EXPECT_TRUE(!res->truncated());
  EXPECT_EQ(2, res->size);

  auto rc = receiver.send(zmq::buffer(str, 2));
  EXPECT_EQ(2, *rc);
}

TEST(ZMQ_REQREP, SendResponseWithBuffer) {
  zmq::context_t context;
  zmq::socket_t sender{context, zmq::socket_type::req};
  zmq::socket_t receiver{context, zmq::socket_type::rep};

  receiver.bind("tcp://*:1234");
  sender.connect("tcp://127.0.0.1:1234");

  const char *str = "Hi";
  EXPECT_EQ(2, *sender.send(zmq::buffer(str, 2)));

  char buf[2];
  const auto res = receiver.recv(zmq::buffer(buf));
  EXPECT_TRUE(res);
  EXPECT_TRUE(!res->truncated());
  EXPECT_EQ(2, res->size);

  auto rc = receiver.send(zmq::buffer(str, 2));
  EXPECT_EQ(2, *rc);
}

TEST(ZMQ_REQREP, BasicRecv) {
  zmq::context_t context;
  zmq::socket_t s{context, zmq::socket_type::req};
  zmq::socket_t s2{context, zmq::socket_type::rep};
  s2.bind("inproc://test");
  s.connect("inproc://test");

  zmq::message_t smsg(size_t{10});
  const auto res_send = s.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  zmq::message_t rmsg;
  const auto res = s2.recv(rmsg, zmq::recv_flags::none);
  EXPECT_TRUE(res);
}

TEST(ZMQ_REQREP, SendResponse) {
  zmq::context_t context;
  zmq::socket_t sender{context, zmq::socket_type::req};
  zmq::socket_t receiver{context, zmq::socket_type::rep};

  receiver.bind("tcp://*:1234");
  sender.connect("tcp://127.0.0.1:1234");

  zmq::message_t smsg(size_t{10});
  const auto res_send = sender.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  zmq::message_t request;
  constexpr auto flags =
      zmq::recv_flags::none /* | zmq::recv_flags::dontwait */;
  const auto res = receiver.recv(request, flags);
  EXPECT_TRUE(res);

  zmq::message_t smsg2(size_t{10});
  auto rc = receiver.send(smsg2, zmq::send_flags::none);
  EXPECT_EQ(10, *rc);

  // is it available send any msg, after recv data? (not response message)
  zmq::message_t smsg3(size_t{20});

  // --> no. only response
  EXPECT_THROW(receiver.send(smsg3, zmq::send_flags::none), zmq::error_t);
}

TEST(ZMQ_PUBSUB, PubSubAsBroadCast) {
  zmq::context_t context{1};
  zmq::socket_t sender{context, zmq::socket_type::pub};
  zmq::socket_t receiver{context, zmq::socket_type::sub};
  zmq::socket_t receiver2{context, zmq::socket_type::sub};
  receiver.set(zmq::sockopt::subscribe, "");
  receiver2.set(zmq::sockopt::subscribe, "");

  sender.bind("inproc://test");
  receiver.connect("inproc://test");
  receiver2.connect("inproc://test");

  zmq::message_t smsg(size_t{10});
  const auto res_send = sender.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  zmq::message_t push_msg;
  constexpr auto flags = zmq::recv_flags::dontwait;
  const auto res = receiver.recv(push_msg, flags);
  zmq::message_t push_msg2;
  const auto res2 = receiver2.recv(push_msg2, flags);

  EXPECT_TRUE(res == res2);
  EXPECT_TRUE(res);
  EXPECT_TRUE(res2);
}

TEST(ZMQ_PUBSUB, PubSubAsAnyCastByUsingCtx) {
  zmq::context_t context{1};
  zmq::context_t context2{2};

  zmq::socket_t sender{context2, zmq::socket_type::pub};
  zmq::socket_t receiver{context2, zmq::socket_type::sub};
  zmq::socket_t receiver2{context, zmq::socket_type::sub};
  receiver.set(zmq::sockopt::subscribe, "");
  receiver2.set(zmq::sockopt::subscribe, "");

  sender.bind("inproc://test");
  receiver.connect("inproc://test");
  receiver2.connect("inproc://test");

  zmq::message_t smsg(size_t{10});
  const auto res_send = sender.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  zmq::message_t push_msg;
  constexpr auto flags = zmq::recv_flags::dontwait;
  const auto res = receiver.recv(push_msg, flags);
  zmq::message_t push_msg2;
  const auto res2 = receiver2.recv(push_msg2, flags);

  EXPECT_TRUE(res != res2);
  EXPECT_TRUE(res);
  EXPECT_TRUE(!res2);
}

TEST(ZMQ_PUBSUB, TopicRecv) {
  zmq::context_t context{1};
  std::string weather = "weather";

  zmq::socket_t sender{context, zmq::socket_type::pub};
  zmq::socket_t receiver{context, zmq::socket_type::sub};
  zmq::socket_t receiver2{context, zmq::socket_type::sub};
  receiver.set(zmq::sockopt::subscribe, "");
  receiver2.set(zmq::sockopt::subscribe, weather);

  sender.bind("inproc://test");
  receiver.connect("inproc://test");
  receiver2.connect("inproc://test");

  const auto topic_send =
      sender.send(zmq::message_t{weather}, zmq::send_flags::sndmore);
  EXPECT_TRUE(topic_send);
  EXPECT_EQ(weather.length(), *topic_send);
  zmq::message_t smsg(size_t{10});
  const auto res_send = sender.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  constexpr auto flags = zmq::recv_flags::none;
  std::vector<zmq::message_t> recv_msgs;
  zmq::recv_result_t result =
      zmq::recv_multipart(receiver, std::back_inserter(recv_msgs));
  EXPECT_TRUE(result);
  EXPECT_EQ(2, *result);
  EXPECT_EQ(weather, recv_msgs[0].to_string());
  EXPECT_EQ(10, recv_msgs[1].size());

  std::vector<zmq::message_t> recv_msgs2;
  zmq::recv_result_t result2 =
      zmq::recv_multipart(receiver2, std::back_inserter(recv_msgs2));
  EXPECT_TRUE(result2);
  EXPECT_EQ(2, *result2);
  EXPECT_EQ(weather, recv_msgs2[0].to_string());
  EXPECT_EQ(10, recv_msgs2[1].size());
}

TEST(ZMQ_PUBSUB, TopicFiltered) {
  zmq::context_t context{1};
  std::string weather = "weather";
  std::string ignored = "ignored";

  zmq::socket_t sender{context, zmq::socket_type::pub};
  zmq::socket_t receiver{context, zmq::socket_type::sub};
  zmq::socket_t receiver2{context, zmq::socket_type::sub};
  receiver.set(zmq::sockopt::subscribe, "");
  receiver2.set(zmq::sockopt::subscribe, weather);

  sender.bind("inproc://test");
  receiver.connect("inproc://test");
  receiver2.connect("inproc://test");

  const auto topic_send =
      sender.send(zmq::message_t{ignored}, zmq::send_flags::sndmore);
  EXPECT_TRUE(topic_send);
  EXPECT_EQ(weather.length(), *topic_send);
  zmq::message_t smsg(size_t{10});
  const auto res_send = sender.send(smsg, zmq::send_flags::none);
  EXPECT_TRUE(res_send);
  EXPECT_EQ(10, *res_send);
  EXPECT_EQ(0, smsg.size());

  constexpr auto flags = zmq::recv_flags::none;
  std::vector<zmq::message_t> recv_msgs;
  zmq::recv_result_t result = zmq::recv_multipart(
      receiver, std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
  EXPECT_TRUE(result);
  EXPECT_EQ(2, *result);
  EXPECT_EQ(ignored, recv_msgs[0].to_string());
  EXPECT_EQ(10, recv_msgs[1].size());

  std::vector<zmq::message_t> recv_msgs2;
  zmq::recv_result_t result2 = zmq::recv_multipart(
      receiver2, std::back_inserter(recv_msgs2), zmq::recv_flags::dontwait);
  EXPECT_TRUE(!result2);
  // it was garbage value
  // EXPECT_EQ(0, *result2);
}
