
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <string>

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

// TODO(@nolleh) refactor
// 1.separate client <-> state
// 2.separate base <-> init
class state_init : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket) {
    return std::shared_ptr<base_state>(new state_init(socket));
  }

  ~state_init() {}

  void send_login(const std::string& uid) {
    ReqLogin login;
    login.set_uid(uid);
    auto message = rsp::libs::message::serializer::serialize(
        MessageType::kReqLogin, login);
    try {
      socket_->send(boost::asio::buffer(message));
    } catch (const std::exception& e) {
      logger_.warn() << "send exception, possible: peer closed:" << e.what()
                     << lg::L_endl;
      close();
      next_ = State::kExit;
    }
  }

  void handle_res_login(buffer_ptr buffer, link*) {
    ResLogin login;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &login)) {
      logger_.error() << "failed to parse reslogin" << lg::L_endl;
      return;
    }

    logger_.info() << "success to login:" << login.uid() << lg::L_endl;
    next_ = State::kLoggedIn;
  }

  virtual void init() {
    std::string uid;
    prompt_ << "type user name to login";
    std::cout << "> ";
    std::cin >> uid;
    send_login(uid);
  }

 protected:
  explicit state_init(socket* socket) : base_state(socket) {
    state_ = State::kInit;
    next_ = state_;
    dispatcher_.register_handler(
        MessageType::kResLogin,
        std::bind(&state_init::handle_res_login, this, std::placeholders::_1,
                  std::placeholders::_2));
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
