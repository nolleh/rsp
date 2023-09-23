#pragma once
#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"

namespace rsp {
namespace cli {
namespace state {

enum class State {
  INIT,
  LOGGED_IN,
  IN_ROOM,
};

const size_t MESSAGE_TYPE_LEN = 1;

// TODO(@nolleh) refactor
class base_state {
 public:
  State state = State::INIT;

  base_state* handle_buffer(std::array<char, 128> buf, size_t num) {
    if (reading_payload_) {
      auto remain = std::vector<char>(buf.begin(), buf.begin() + num);
      return handle_payload(reading_payload_, remain, num);
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
      return handle_payload(msgType, remain, num - remain_for_type);
    }
    return this;
  }

  base_state* handle_payload(MessageType type, std::vector<char> buf, int num) {
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
  std::vector<char> buf_;
  MessageType reading_payload_;
};

// template <>
// base_state* base_state::handle_message(ResLogin&& login);

}  // namespace state
}  // namespace cli
}  // namespace rsp
