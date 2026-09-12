/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "proto/common/message_type.pb.h"
#include "proto/common/ping.pb.h"
#include "rspcli/prompt/prompt.hpp"
#include "rspcli/state/context.hpp"
#include "rsplib/logger/logger.hpp"
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

using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
using message_sender = std::function<void(MessageType, raw_buffer)>;
using transition = std::optional<State>;
namespace lg = libs::logger;

class base_state {
 public:
  virtual ~base_state() = default;

  virtual void enter() {}
  virtual transition on_command(std::string_view command) = 0;

  virtual transition on_message(MessageType type, buffer_ptr payload) {
    if (type == MessageType::kPing) {
      send_message(MessageType::kPong, Pong{});
      return std::nullopt;
    }

    logger_.warn() << "unknown message type: " << static_cast<int>(type)
                   << lg::L_endl;
    return State::kExit;
  }

  friend std::ostream& operator<<(std::ostream&, const base_state&);

 protected:
  explicit base_state(context* context, message_sender sender)
      : context_(context),
        sender_(std::move(sender)),
        logger_(lg::logger()),
        prompt_(this) {}

  template <typename T>
  void send_message(MessageType type, const T& message) {
    sender_(type, rsp::libs::message::serializer::serialize(type, message));
  }

  uint64_t get_request_id() { return request_id_++; }

  void show_prompt(const char* message) {
    prompt_ << message;
    std::cout << "> " << std::flush;
  }

  State state_ = State::kInit;
  context* context_;
  message_sender sender_;
  lg::s_logger& logger_;
  prompt<base_state> prompt_;

 private:
  uint64_t request_id_ = 0;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
