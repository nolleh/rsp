
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_in_room : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket) {
    return std::shared_ptr<state_in_room>(new state_in_room(socket));
  }

  ~state_in_room() { dispatcher_.unregister_handler(MessageType::kResLogout); }

  void init() override {
    // TODO(@nolleh) does dispatcher really need a ability
    // register same message type with different instance? hum...
    // client state is changed, and old state is erased after newer one,
    // registered type is accidentally erased issue is there.
    // so until mind is arranged, used redundant register
    // dispatcher_.register_handler(
    //     MessageType::kResLogout,
    //     std::bind(&state_in_room::handle_res_logout, this,
    //               std::placeholders::_1, std::placeholders::_2));
    // std::string commands[]{"logout", "create_room", "join_room"};
    //
    // auto command_direction = join(',', commands);
    // prompt_ << std::format("possible command \n{}\n", command_direction);
    prompt_ << "possible command \n1) logout 2)read message\n 3)send message";
    std::cout << "> ";
    std::string command;
    std::cin >> command;

    switch (std::stoi(command)) {
      case 1:
        send_message(MessageType::kReqLogout, ReqLogout{});
        break;
      case 2:
        init();
        break;
      case 3:
        prompt_ << "type message to send";
        std::cout << "> ";
        std::string msg;
        std::cin >> msg;
        ReqFwdRoom fwd;
        fwd.set_message(msg);
        send_message(MessageType::kReqFwdRoom, fwd);
        break;
    }
  }

 protected:
  explicit state_in_room(socket* socket) : base_state(socket) {
    state_ = State::kInRoom;
    next_ = state_;
    dispatcher_.register_handler(
        MessageType::kResLogout,
        std::bind(&state_in_room::handle_res_logout, this,
                  std::placeholders::_1, std::placeholders::_2));
    dispatcher_.register_handler(
        MessageType::kResFwdRoom,
        std::bind(&state_in_room::handle_res_fwd_room, this,
                  std::placeholders::_1, std::placeholders::_2));
  }

 private:
  void handle_res_logout(buffer_ptr buffer, link*) {
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

  void handle_res_fwd_room(buffer_ptr buffer, link*) {
    ResFwdRoom fwd_room;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &fwd_room)) {
      logger_.error() << "failed to fwd room" << lg::L_endl;
      return;
    }

    logger_.debug() << "successfully sent message: " << fwd_room.message()
                    << lg::L_endl;
  }

  template <typename T>
  void send_message(MessageType type, T&& msg) {
    auto message = rsp::libs::message::serializer::serialize(type, msg);
    try {
      socket_->send(boost::asio::buffer(message));
    } catch (const std::exception& e) {
      logger_.warn() << "send exception, possible: peer closed:" << e.what()
                     << lg::L_endl;
      close();
      next_ = State::kExit;
    }
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
