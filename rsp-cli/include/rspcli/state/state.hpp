/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "proto/common/message_type.pb.h"
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
using buffer_ptr = libs::message::buffer_ptr;
namespace lg = libs::logger;

// TODO(@nolleh) refactor
// 1.seperate client <-> state
// 2.seperate base <-> init
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
    // and, for convient dev, remove by type for now
    dispatcher_.unregister_handler(MessageType::kResLogin);
  }

  void send_login(const std::string& uid) {
    ReqLogin login;
    login.set_uid(uid);
    auto message = rsp::libs::message::serializer::serialize(
        MessageType::kReqLogin, login);
    socket_->send(boost::asio::buffer(message));
  }

  State handle_buffer(std::array<char, 128> buf, size_t len) {
    interpreter_.handle_buffer(buf, len);
    return next_;
  }

  void handle_res_login(buffer_ptr buffer) {
    ResLogin login;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &login)) {
      logger_.error() << "failed to parse reslogin" << lg::L_endl;
      return;
    }

    logger_.info() << "success to login:" << login.uid() << lg::L_endl;
    next_ = State::kLoggedIn;
  }

  virtual void init() {
    dispatcher_.register_handler(
        MessageType::kResLogin,
        std::bind(&base_state::handle_res_login, this, std::placeholders::_1));

    std::string uid;
    prompt_ << "type user name to login";
    std::cout << "> ";
    std::cin >> uid;
    send_login(uid);
  }

  friend std::ostream& operator<<(std::ostream&, const base_state&);

  void close() {
    namespace asio = boost::asio::ip;
    boost::system::error_code shutdown_ec;
    socket_->shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    if (shutdown_ec)
      logger_.error() << "shutdown error" << shutdown_ec << ":"
                      << shutdown_ec.message() << lg::L_endl;
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

 protected:
  explicit base_state(socket* socket)
      : socket_(socket),
        dispatcher_(message_dispatcher::instance()),
        interpreter_(&dispatcher_),
        logger_(lg::logger()),
        prompt_(this) {}

  State state_ = State::kInit;
  socket* socket_;
  message_dispatcher& dispatcher_;
  State next_;
  lg::s_logger& logger_;
  prompt<base_state> prompt_;
  bool sent_shutdown_;

 private:
  conn_interpreter interpreter_;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
