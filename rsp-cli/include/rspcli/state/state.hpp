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
#include "rsplib/message/types.hpp"

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
using shared_const_buffer = libs::buffer::shared_const_buffer;
using shared_mutable_buffer = libs::buffer::shared_mutable_buffer;
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
    std::cout << "type user name to login" << std::endl;
    std::cin >> uid;
    send_login(uid);
  }

  void send_login(const std::string& uid) {
    ReqLogin login;
    login.set_uid(uid);
    auto payload = login.SerializeAsString();
    const auto content_len = login.ByteSizeLong();
    raw_buffer message;
    namespace msg = rsp::libs::message;
    msg::mset(&message, content_len);
    msg::mset(&message, static_cast<int>(MessageType::kReqLogin));
    // std::cout << "message:" << msg::to_string(message) << std::endl;
    logger_.trace() << "type:"
                    << msg::to_string(static_cast<int>(MessageType::kReqLogin))
                    << lg::L_endl;
    message.insert(message.end(), payload.begin(), payload.end());
    logger_.debug() << "message size:" << message.size() << lg::L_endl;
    // std::cout << "message:" << msg::to_string(message) << std::endl;
    socket_->send(boost::asio::buffer(message));
  }

  base_state* handle_buffer(shared_mutable_buffer buf, size_t len) {
    logger_.debug() << "handle_buffer" << len << "," << buf.end() - buf.begin()
                    << "," << buf.data_end() - buf.data_begin() << lg::L_endl;
    // interpreter_.handle_buffer(buf);
    return this;
  }

  base_state* handle_buffer(std::array<char, 128> buf, size_t len) {
    interpreter_.handle_buffer(buf, len);
    return this;
  }

  void handle_reslogin(buffer_ptr buffer) {
    ResLogin login;
    std::string str{buffer->begin(), buffer->end()};
    if (!login.ParseFromString(str)) {
      logger_.error() << "failed to parse reslogin" << lg::L_endl;
    }

    if (!login.success()) {
      logger_.error() << "failed to login (" << login.uid()
                      << "), bytes: " << login.ByteSizeLong() << lg::L_endl;
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
