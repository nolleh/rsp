/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "proto/user/login.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_init : public base_state {
 public:
  static std::unique_ptr<base_state> create(context* context,
                                            message_sender sender) {
    return std::unique_ptr<state_init>(
        new state_init(context, std::move(sender)));
  }

  void enter() override { show_prompt("type user name to login"); }

  transition on_command(std::string_view command) override {
    ReqLogin login;
    login.set_request_id(get_request_id());
    login.set_uid(std::string(command));
    send_message(MessageType::kReqLogin, login);
    return std::nullopt;
  }

  transition on_message(MessageType type, buffer_ptr payload) override {
    if (type != MessageType::kResLogin) {
      return base_state::on_message(type, std::move(payload));
    }

    ResLogin login;
    if (!rsp::libs::message::serializer::deserialize(*payload, &login)) {
      logger_.error() << "failed to parse reslogin" << lg::L_endl;
      return std::nullopt;
    }

    logger_.info() << "success to login:" << login.uid() << lg::L_endl;
    context_->uid = login.uid();
    return State::kLoggedIn;
  }

 private:
  explicit state_init(context* context, message_sender sender)
      : base_state(context, std::move(sender)) {
    state_ = State::kInit;
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
