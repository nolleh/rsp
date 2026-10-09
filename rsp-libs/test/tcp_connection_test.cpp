/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rsplib/server/tcp_connection.hpp"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <future>
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

  std::promise<void> accepted;
  auto accepted_result = accepted.get_future();
  tcp::acceptor acceptor{io_context, tcp::endpoint(tcp::v4(), 0)};
  acceptor.async_accept(
      connection->socket(),
      [&accepted](const boost::system::error_code& error) {
        if (!error) accepted.set_value();
      });

  tcp::socket client{io_context};
  client.connect(acceptor.local_endpoint());

  auto work = asio::make_work_guard(io_context);
  std::thread runner([&io_context] { io_context.run(); });

  const auto accept_status =
      accepted_result.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(std::future_status::ready, accept_status);
  if (accept_status != std::future_status::ready) {
    work.reset();
    io_context.stop();
    runner.join();
    return;
  }

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

TEST(TcpConnection, NotifiesCloseOnceAfterPeerDisconnects) {
  namespace asio = boost::asio;
  using tcp = asio::ip::tcp;

  asio::io_context io_context;
  rsp::libs::message::message_dispatcher_interface dispatcher;
  std::atomic<int> close_count{0};
  std::promise<void> closed;
  auto closed_result = closed.get_future();
  auto connection = rsp::libs::server::tcp_connection::create(
      &io_context, &dispatcher,
      [&close_count, &closed](
          const rsp::libs::server::connection_ptr& connection) {
        if (close_count.fetch_add(1) == 0) closed.set_value();
        connection->stop(true);
      });

  tcp::acceptor acceptor{io_context, tcp::endpoint(tcp::v4(), 0)};
  acceptor.async_accept(
      connection->socket(),
      [connection](const boost::system::error_code& error) {
        if (!error) {
          connection->start(rsp::libs::server::tcp_connection::kBufBytes);
        }
      });

  tcp::socket client{io_context};
  client.connect(acceptor.local_endpoint());

  auto work = asio::make_work_guard(io_context);
  std::thread runner([&io_context] { io_context.run(); });

  boost::system::error_code ignored;
  client.shutdown(tcp::socket::shutdown_both, ignored);
  client.close(ignored);

  const auto close_status = closed_result.wait_for(std::chrono::seconds(1));
  if (close_status != std::future_status::ready) connection->stop(true);

  EXPECT_EQ(std::future_status::ready, close_status);
  EXPECT_EQ(1, close_count.load());

  work.reset();
  io_context.stop();
  runner.join();
}

TEST(TcpConnection, GracefulStopDrainsQueuedMessagesBeforeEof) {
  namespace asio = boost::asio;
  using tcp = asio::ip::tcp;

  asio::io_context io_context;
  rsp::libs::message::message_dispatcher_interface dispatcher;
  auto connection =
      rsp::libs::server::tcp_connection::create(&io_context, &dispatcher);

  std::promise<void> accepted;
  auto accepted_result = accepted.get_future();
  tcp::acceptor acceptor{io_context, tcp::endpoint(tcp::v4(), 0)};
  acceptor.async_accept(
      connection->socket(),
      [&accepted, connection](const boost::system::error_code& error) {
        if (!error) {
          connection->socket().set_option(
              asio::socket_base::send_buffer_size(1024));
          accepted.set_value();
        }
      });

  tcp::socket client{io_context};
  client.connect(acceptor.local_endpoint());
  auto work = asio::make_work_guard(io_context);
  std::thread runner([&io_context] { io_context.run(); });

  const auto accept_status = accepted_result.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(std::future_status::ready, accept_status);
  if (accept_status != std::future_status::ready) {
    work.reset();
    io_context.stop();
    runner.join();
    return;
  }

  std::string received;
  std::promise<boost::system::error_code> completed;
  auto result = completed.get_future();
  asio::async_read(client, asio::dynamic_buffer(received),
                   [&completed](const boost::system::error_code& error,
                                std::size_t) { completed.set_value(error); });

  constexpr std::size_t kMessageSize = 64 * 1024;
  connection->send(std::vector<char>(kMessageSize, 'a'));
  connection->send(std::vector<char>(kMessageSize, 'b'));
  connection->stop(false);

  const auto status = result.wait_for(std::chrono::seconds(5));
  EXPECT_EQ(std::future_status::ready, status);
  if (status == std::future_status::ready) {
    EXPECT_EQ(asio::error::eof, result.get());
    EXPECT_EQ(std::string(kMessageSize, 'a') + std::string(kMessageSize, 'b'),
              received);
  }

  connection->stop(true);
  work.reset();
  io_context.stop();
  runner.join();
}
