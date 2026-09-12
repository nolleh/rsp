/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rsplib/server/tcp_connection.hpp"

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "rsplib/message/message_dispatcher_interface.hpp"

TEST(TcpConnection, WritesQueuedMessagesInOrder) {
  namespace asio = boost::asio;
  using tcp = asio::ip::tcp;

  asio::io_context io_context;
  rsp::libs::message::message_dispatcher_interface dispatcher;
  auto connection =
      rsp::libs::server::tcp_connection::create(&io_context, &dispatcher);

  tcp::acceptor acceptor{io_context, tcp::endpoint(tcp::v4(), 0)};
  acceptor.async_accept(connection->socket(),
                        [](const boost::system::error_code&) {});

  tcp::socket client{io_context};
  client.connect(acceptor.local_endpoint());

  auto work = asio::make_work_guard(io_context);
  std::thread runner([&io_context] { io_context.run(); });

  connection->send(std::vector<char>{'f', 'i', 'r', 's', 't'});
  connection->send(std::vector<char>{'s', 'e', 'c', 'o', 'n', 'd'});

  std::array<char, 11> received{};
  asio::read(client, asio::buffer(received));

  EXPECT_EQ("firstsecond", std::string(received.begin(), received.end()));

  connection->stop(true);
  work.reset();
  io_context.stop();
  runner.join();
}
