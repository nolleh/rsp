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
#include "rsplib/message/message_dispatcher.hpp"

namespace rsp {
namespace cli {
namespace state {

enum class State {
  INIT,
  LOGGED_IN,
  IN_ROOM,
};

const size_t MESSAGE_TYPE_LEN = 1;

using socket = boost::asio::ip::tcp::socket;
using conn_interpreter = libs::message::conn_interpreter;
using message_dispatcher = libs::message::message_dispatcher;
using shared_const_buffer = libs::buffer::shared_const_buffer;
using shared_mutable_buffer = libs::buffer::shared_mutable_buffer;
using logger = libs::logger::logger;

// TODO(@nolleh) refactor
class base_state {
 public:
  State state = State::INIT;
  explicit base_state(socket* socket)
      : socket_(socket),
        dispatcher_(message_dispatcher::instance()),
        logger_(logger::instance()) {
    // send_login();
    dispatcher_.register_handler(
        MessageType::RES_LOGIN,
        std::bind(&base_state::handle_reslogin, this, std::placeholders::_1));
  }

  void send_login() {
    ReqLogin login;
    login.set_uid("nolleh");
    auto payload = login.SerializeAsString();
    const auto content_len = std::to_string(login.ByteSizeLong());
    std::vector<char> message{content_len.begin(), content_len.end()};
    message.emplace_back(MessageType::REQ_LOGIN);
    message.insert(message.end(), payload.begin(), payload.end());
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

  void handle_reslogin(std::shared_ptr<std::vector<char>> buffer) {
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

  base_state* handle_buffer_old(std::array<char, 128> buf, size_t num) {
    if (reading_payload_) {
      auto remain = std::vector<char>(buf.begin(), buf.begin() + num);
      return handle_payload_old(reading_payload_, remain, num);
    }

    // retreiving type
    if (num < MESSAGE_TYPE_LEN) {
      // partialbuffer. remember
      // buf_.emplace_back(buf.begin(), buf.begin() + num);
      return this;
    }
    buf_.clear();
    std::cout << "handle_buffer, read:" << num << std::endl;
    const size_t remain_for_type = MESSAGE_TYPE_LEN - buf_.size();
    std::string type{buf.begin(), buf.begin() + remain_for_type};
    auto msgType = static_cast<MessageType>(stoi(type));
    reading_payload_ = msgType;
    if (num - remain_for_type > 0) {
      auto remain =
          std::vector<char>(buf.begin() + remain_for_type, buf.begin() + num);
      return handle_payload_old(msgType, remain, num - remain_for_type);
    }
    return this;
  }

  base_state* handle_payload_old(MessageType type, std::vector<char> buf,
                                 int num) {
    std::cout << "handle_payload" << num << std::endl;
    // proto buf calculate the size of message internally.
    // looks like not optimized way...
    //
    if (MessageType::RES_LOGIN == type) {
      ResLogin msg;
      std::string str{buf.begin(), buf.end()};
      if (!msg.ParseFromString(str)) {
        std::cerr << "failed to parse message" << std::endl;
        // change to error state and disconn
        return this;
      }

      if (!msg.success()) {
        std::cerr << "failed to login (" << msg.uid()
                  << "), bytes: " << msg.ByteSizeLong() << std::endl;
        return this;
      }
      std::cout << "success to login:" << msg.uid() << std::endl;
      // TODO(@nolleh) make factory
      // return static_cast<base_state*>(new state_login);
      return this;
    }
    return this;
  }

  template <typename Message>
  base_state* handle_message(Message&& message) {
    std::cout << "init state handleMessage" << std::endl;
    return this;
  }

 private:
  socket* socket_;
  std::vector<char> buf_;
  MessageType reading_payload_;
  conn_interpreter interpreter_;
  message_dispatcher& dispatcher_;
  logger& logger_;
};

// template <>
// base_state* base_state::handle_message(ResLogin&& login);

}  // namespace state
}  // namespace cli
}  // namespace rsp
