/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_login : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket) {
    return std::shared_ptr<state_login>(new state_login(socket));
  }

  ~state_login() { dispatcher_.unregister_handler(MessageType::kResLogout); }

  void init() override {
    dispatcher_.register_handler(MessageType::kResLogout,
                                 std::bind(&state_login::handle_res_logout,
                                           this, std::placeholders::_1));

    std::string command;
    prompt_ << "possible command \n1) logout";
    std::cout << "> ";
    std::cin >> command;

    if (command == "1") {
      send_logout();
    }
  }

 protected:
  explicit state_login(socket* socket) : base_state(socket) {
    state_ = State::kLoggedIn;
  }

 private:
  void handle_res_logout(buffer_ptr buffer) {
    ResLogout logout;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &logout)) {
      logger_.error() << "failed to parse logout" << lg::L_endl;
      return;
    }

    logger_.info() << "success to logout, bye bye:" << logout.uid()
                   << lg::L_endl;
    close();
    next_ = State::kExit;
  }

  void send_logout() {
    ReqLogout logout;
    auto message = rsp::libs::message::serializer::serialize(
        MessageType::kReqLogout, logout);
    socket_->send(boost::asio::buffer(message));
  }
};
}  // namespace state
}  // namespace cli
}  // namespace rsp
