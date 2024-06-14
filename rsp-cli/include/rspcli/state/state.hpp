/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "proto/common/message_type.pb.h"
#include "proto/common/ping.pb.h"
#include "proto/user/login.pb.h"
#include "rspcli/prompt/prompt.hpp"
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace cli {
namespace state {

enum class State {
  kInit,
  kLoggedIn,
  kInRoom,
  kExit,
};

using socket = boost::asio::ip::tcp::socket;
using conn_interpreter = libs::message::conn_interpreter;
using message_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using link = libs::message::link;
using buffer_ptr = libs::message::buffer_ptr;
namespace lg = libs::logger;

// TODO(@nolleh) refactor
// 1.separate client <-> state
// 2.separate base <-> init
class base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket) {
    return std::shared_ptr<base_state>(new base_state(socket));
  }

  ~base_state() {
    // REMARK(@nolleh) register/unregister per state is not good pattern,
    // but client has only one conn/state,
    // this is okay to use like this.
    // if this precondition is not effective, then change the implementation as
    // server pattern.
    // and, for convenient dev, remove by type for now
    dispatcher_.unregister_handler(MessageType::kResLogin);
    dispatcher_.unregister_handler(MessageType::kPing);
  }

  void send_pong() {
    Pong pong;
    auto message =
        rsp::libs::message::serializer::serialize(MessageType::kPong, pong);
    try {
      socket_->send(boost::asio::buffer(message));
    } catch (const std::exception& e) {
      logger_.warn() << "send exception, possible: peer closed:" << e.what()
                     << lg::L_endl;
      close();
      next_ = State::kExit;
    }
  }

  State handle_buffer(std::array<char, 128> buf, size_t len) {
    interpreter_.handle_buffer(buf, len);
    return next_;
  }

  void handle_ping(buffer_ptr buffer, link*) { send_pong(); }

  void handle_unknown(link*) {
    logger_.warn() << "unknown message" << lg::L_endl;
    close();
    next_ = State::kExit;
  }

  virtual void init() {}

  friend std::ostream& operator<<(std::ostream&, const base_state&);

  void close() {
    namespace asio = boost::asio::ip;
    boost::system::error_code shutdown_ec;
    socket_->shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    if (shutdown_ec) {
      logger_.info() << "shutdown error" << shutdown_ec << ":"
                     << shutdown_ec.message() << lg::L_endl;
    }
    sent_shutdown_ = true;
    logger_.debug() << "activate close" << lg::L_endl;
  }

  void close(boost::system::error_code ec) {
    namespace asio = boost::asio::ip;
    if (sent_shutdown_) {
      socket_->close();
      return;
    }
    logger_.debug() << "ec" << ec.message() << ", shutdown" << lg::L_endl;
    boost::system::error_code shutdown_ec;
    socket_->shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    if (shutdown_ec)
      logger_.error() << "shutdown error" << shutdown_ec << ":"
                      << shutdown_ec.message() << lg::L_endl;
    sent_shutdown_ = true;
    socket_->close();
  }

  template <typename SyncReadStream>
  void read_with_timeout(
      SyncReadStream* s,
      const boost::asio::deadline_timer::duration_type& expiry_time) {
    boost::asio::deadline_timer timer(s->get_executor());
    timer.expires_from_now(expiry_time);
    timer.async_wait(
        [&s](const boost::system::error_code& error) { s->cancel(); });

    auto ptr =
        std::shared_ptr<std::array<char, 128>>(new std::array<char, 128>);
    boost::asio::async_read(
        *s, boost::asio::buffer(*ptr),
        [this, ptr, &timer](const boost::system::error_code& error,
                            size_t size) {
          timer.cancel();
          handle_buffer(*ptr, size);
        });
  }

 protected:
  explicit base_state(socket* socket)
      : socket_(socket),
        dispatcher_(message_dispatcher::instance()),
        interpreter_(&dispatcher_),
        logger_(lg::logger()),
        prompt_(this) {
    dispatcher_.register_unknown_message_handler(
        std::bind(&base_state::handle_unknown, this, nullptr));
    dispatcher_.register_handler(
        MessageType::kPing,
        std::bind(&base_state::handle_ping, this, std::placeholders::_1,
                  std::placeholders::_2));
  }

  uint64_t get_request_id() { return request_id_++; }

  State state_ = State::kInit;
  State next_ = State::kInit;
  socket* socket_;
  message_dispatcher& dispatcher_;
  lg::s_logger& logger_;
  prompt<base_state> prompt_;
  bool sent_shutdown_;

 private:
  conn_interpreter interpreter_;
  uint64_t request_id_;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
