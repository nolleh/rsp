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
