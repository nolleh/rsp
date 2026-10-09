/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
// SPDX-License-Identifier: Apache-2.0

#include "rsplib/server/tcp_server.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include "proto/user/login.pb.h"
#include "rsplib/link/link.hpp"
#include "rsplib/message/serializer.hpp"

namespace {

class test_session : public rsp::libs::link::link {
 public:
  explicit test_session(const rsp::libs::server::connection_ptr& connection)
      : link(connection) {}
  void on_connected() override {}
  void on_disconnected() override {}
  void on_closed() override {}
};

class delayed_session_owner : public rsp::libs::server::server_event {
 public:
  void on_conn_created(
      const rsp::libs::server::connection_ptr& connection) override {
    // Keep session setup busy while a complete request is already buffered.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session_ = std::make_unique<test_session>(connection);
    connection->attach_link(session_.get());
  }

  void on_conn_closed(const rsp::libs::server::connection_ptr&) override {}

 private:
  std::unique_ptr<test_session> session_;
};

class session_check_dispatcher
    : public rsp::libs::message::message_dispatcher_interface {
 public:
  void dispatch(MessageType, const rsp::libs::message::raw_buffer&,
                rsp::libs::link::link* session) override {
    std::_Exit(session ? 0 : 3);
  }
};

}  // namespace

TEST(TcpServerDeathTest, InitializesSessionBeforeReadingBufferedRequest) {
  EXPECT_EXIT(
      ([] {
        namespace asio = boost::asio;
        using tcp = asio::ip::tcp;

        // Isolate the blocking server and fail instead of hanging on regression.
        std::thread([] {
          std::this_thread::sleep_for(std::chrono::seconds(3));
          std::_Exit(2);
        }).detach();

        session_check_dispatcher dispatcher;
        delayed_session_owner owner;
        rsp::libs::server::tcp_server server(&dispatcher, 0);
        server.subscribe(&owner);

        asio::io_context client_context;
        tcp::socket client(client_context);
        client.connect(tcp::endpoint(asio::ip::address_v4::loopback(),
                                    server.local_endpoint().port()));
        ReqLogin login;
        login.set_uid("early-login");
        const auto request = rsp::libs::message::serializer::serialize(
            MessageType::kReqLogin, login);
        asio::write(client, asio::buffer(request));

        // Accept with the request already queued, before session setup begins.
        server.start();
        std::_Exit(4);
      }()),
      ::testing::ExitedWithCode(0), "");
}
