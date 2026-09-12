/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/client_session.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rsplib/message/serializer.hpp"

namespace rsp::cli {
namespace {

template <typename Message>
void send_message(boost::asio::ip::tcp::socket& socket, MessageType type,
                  const Message& message) {
  const auto buffer = libs::message::serializer::serialize(type, message);
  boost::asio::write(socket, boost::asio::buffer(buffer));
}

void run_until_state(boost::asio::io_context& io_context,
                     const std::shared_ptr<client_session>& client,
                     state::State expected) {
  for (int attempt = 0; attempt < 10; ++attempt) {
    io_context.restart();
    io_context.run_for(std::chrono::milliseconds(10));
    if (client->current_state() == expected) return;
  }
}

TEST(ClientSession, ReturnsToLoggedInAfterLeaveResponse) {
  using tcp = boost::asio::ip::tcp;

  boost::asio::io_context io_context;
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 0));
  tcp::socket server_socket(io_context);
  server_socket.connect(acceptor.local_endpoint());
  tcp::socket client_socket = acceptor.accept();

  auto client = std::make_shared<client_session>(std::move(client_socket));
  client->start();

  client->handle_command("nolleh");
  ResLogin login;
  login.set_uid("nolleh");
  send_message(server_socket, MessageType::kResLogin, login);
  run_until_state(io_context, client, state::State::kLoggedIn);
  ASSERT_EQ(client->current_state(), state::State::kLoggedIn);

  client->handle_command("2");
  ResCreateRoom create_room;
  create_room.set_room_id(42);
  send_message(server_socket, MessageType::kResCreateRoom, create_room);
  run_until_state(io_context, client, state::State::kInRoom);
  ASSERT_EQ(client->current_state(), state::State::kInRoom);

  client->handle_command("3");
  ResLeaveRoom leave_room;
  leave_room.set_success(true);
  send_message(server_socket, MessageType::kResLeaveRoom, leave_room);
  run_until_state(io_context, client, state::State::kLoggedIn);

  EXPECT_EQ(client->current_state(), state::State::kLoggedIn);
  client->stop();
}

}  // namespace
}  // namespace rsp::cli
