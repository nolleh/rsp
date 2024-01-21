
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
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
    dispatcher_.register_handler(
        MessageType::kResLogout,
        std::bind(&state_in_room::handle_res_logout, this,
                  std::placeholders::_1, std::placeholders::_2));

    // std::string commands[]{"logout", "create_room", "join_room"};
    //
    // auto command_direction = join(',', commands);
    // prompt_ << std::format("possible command \n{}\n", command_direction);
    prompt_ << "possible command \n1) logout\n";
    std::cout << "> ";
    std::string command;
    std::cin >> command;

    if (command == "1") {
      send_message<ReqLogout>(MessageType::kReqLogout);
    }
  }

 protected:
  explicit state_in_room(socket* socket) : base_state(socket) {
    state_ = State::kInRoom;
    next_ = state_;
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

  template <typename T>
  void send_message(MessageType type) {
    T logout;
    auto message = rsp::libs::message::serializer::serialize(type, logout);
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
