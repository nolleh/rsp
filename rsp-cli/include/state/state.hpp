#pragma once
#include <array>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
  INIT,
  LOGGED_IN,
  IN_ROOM,
};

using socket = boost::asio::ip::tcp::socket;
using conn_interpreter = libs::message::conn_interpreter;
using message_dispatcher = libs::message::message_dispatcher;
using shared_const_buffer = libs::buffer::shared_const_buffer;
using shared_mutable_buffer = libs::buffer::shared_mutable_buffer;
using logger = libs::logger::logger;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;

// TODO(@nolleh) refactor
class base_state {
 public:
  State state = State::INIT;
  explicit base_state(socket* socket)
      : socket_(socket),
        dispatcher_(message_dispatcher::instance()),
        interpreter_(&dispatcher_),
        logger_(logger::instance()) {
    // send_login();
    dispatcher_.register_handler(
        MessageType::RES_LOGIN,
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
    msg::mset(&message, static_cast<int>(MessageType::REQ_LOGIN));
    // std::cout << "message:" << msg::to_string(message) << std::endl;
    std::cout << "type:"
              << msg::to_string(static_cast<int>(MessageType::REQ_LOGIN))
              << std::endl;
    message.insert(message.end(), payload.begin(), payload.end());
    std::cout << "message size:" << message.size() << std::endl;
    // std::cout << "message:" << msg::to_string(message) << std::endl;
    socket_->send(boost::asio::buffer(message));
  }

  base_state* handle_buffer(shared_mutable_buffer buf, size_t len) {
    std::cout << "handle_buffer" << len << "," << buf.end() - buf.begin() << ","
              << buf.data_end() - buf.data_begin() << std::endl;
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
      logger_.error("failed to parse reslogin");
    }

    if (!login.success()) {
      std::cerr << "failed to login (" << login.uid()
                << "), bytes: " << login.ByteSizeLong() << std::endl;
    }
    std::cout << "success to login:" << login.uid() << std::endl;
  }

  template <typename Message>
  base_state* handle_message(Message&& message) {
    std::cout << "init state handleMessage" << std::endl;
    return this;
  }

 private:
  socket* socket_;
  message_dispatcher& dispatcher_;
  conn_interpreter interpreter_;
  logger& logger_;
};

// template <>
// base_state* base_state::handle_message(ResLogin&& login);

}  // namespace state
}  // namespace cli
}  // namespace rsp
