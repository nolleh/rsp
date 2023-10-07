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
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"
#include "rspcli/prompt/prompt.hpp"

namespace rsp {
namespace cli {
namespace state {

enum class State {
  kInit,
  kLoggedIn,
  kInRoom,
};

using socket = boost::asio::ip::tcp::socket;
using conn_interpreter = libs::message::conn_interpreter;
using message_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
namespace lg = libs::logger;

// TODO(@nolleh) refactor
class base_state {
 public:
  State state = State::kInit;
  explicit base_state(socket* socket)
      : socket_(socket),
        dispatcher_(message_dispatcher::instance()),
        interpreter_(&dispatcher_),
        logger_(lg::logger()) {
    dispatcher_.register_handler(
        MessageType::kResLogin,
        std::bind(&base_state::handle_reslogin, this, std::placeholders::_1));

    std::string uid;
    prompt() << "type user name to login";
    std::cin >> uid;
    send_login(uid);
  }

  void send_login(const std::string& uid) {
    ReqLogin login;
    login.set_uid(uid);
    auto message = rsp::libs::message::serializer::serialize(
        MessageType::kReqLogin, login);
    socket_->send(boost::asio::buffer(message));
  }

  base_state* handle_buffer(std::array<char, 128> buf, size_t len) {
    interpreter_.handle_buffer(buf, len);
    return this;
  }

  void handle_reslogin(buffer_ptr buffer) {
    ResLogin login;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &login)) {
      logger_.error() << "failed to parse reslogin" << lg::L_endl;
    }

    logger_.info() << "success to login:" << login.uid() << lg::L_endl;
  }

  template <typename Message>
  base_state* handle_message(Message&& message) {
    logger_.trace() << "init state handleMessage" << lg::L_endl;
    return this;
  }

 private:
  socket* socket_;
  message_dispatcher& dispatcher_;
  conn_interpreter interpreter_;
  lg::s_logger& logger_;
};

// template <>
// base_state* base_state::handle_message(ResLogin&& login);

}  // namespace state
}  // namespace cli
}  // namespace rsp
