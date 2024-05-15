/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/

// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>
#include <zmq.hpp>

TEST(ZMQ_SOCKET, ReqSocketSend) {
  zmq::context_t ctx{};
  zmq::socket_t req{ctx, zmq::socket_type::req};
  req.connect("inproc://test");

  std::string msg{"hello, world"};
  zmq::message_t request{msg};

  auto rc = req.send(request, zmq::send_flags::dontwait);
  EXPECT_EQ(msg.size(), rc);
}

TEST(ZMQ_SOCKET, RepSocketRecv) {
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

TEST(ZMQ_SOCKET, RepSocketSend) {
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

TEST(ZMQ_SOCKET, RepTcpSocketSend) {
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

TEST(ZMQ_SOCKET, ZMQ) {
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

TEST(ZMQ_SOCKET, RepTcpSocketMessage) {
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
  constexpr auto flags = zmq::recv_flags::none /* | zmq::recv_flags::dontwait */;
  const auto res = receiver.recv(request, flags);
  EXPECT_TRUE(res);

  zmq::message_t smsg2(size_t{10});
  auto rc = receiver.send(smsg2, zmq::send_flags::none);
  EXPECT_EQ(10, *rc);
}
