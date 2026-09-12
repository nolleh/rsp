/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "proto/user/login.pb.h"
#include "proto/user/to_client.pb.h"
#include "proto/user/to_room.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_in_room : public base_state {
 public:
  static std::unique_ptr<base_state> create(context* context,
                                            message_sender sender) {
    return std::unique_ptr<state_in_room>(
        new state_in_room(context, std::move(sender)));
  }

  void enter() override {
    pending_input_ = pending_input::kCommand;
    show_prompt(
        "possible command \n1) logout 2) send message 3) leave room "
        "4) kickout");
  }

  transition on_command(std::string_view command) override {
    if (pending_input_ == pending_input::kMessage) {
      FwdRoom forward;
      forward.set_message(std::string(command));
      pending_input_ = pending_input::kCommand;
      send_message(MessageType::kFwdRoom, forward);
      show_command_prompt();
      return std::nullopt;
    }

    if (pending_input_ == pending_input::kKickoutUser) {
      FwdRoom forward;
      forward.set_message("kickout:" + std::string(command));
      pending_input_ = pending_input::kCommand;
      send_message(MessageType::kFwdRoom, forward);
      show_command_prompt();
      return std::nullopt;
    }

    if (command == "1") {
      ReqLogout logout;
      logout.set_request_id(get_request_id());
      send_message(MessageType::kReqLogout, logout);
    } else if (command == "2") {
      pending_input_ = pending_input::kMessage;
      show_prompt("type message to send");
    } else if (command == "3") {
      ReqLeaveRoom leave_room;
      leave_room.set_request_id(get_request_id());
      send_message(MessageType::kReqLeaveRoom, leave_room);
    } else if (command == "4") {
      pending_input_ = pending_input::kKickoutUser;
      show_prompt("type user name to kickout");
    } else {
      show_prompt("your command is incorrect");
    }
    return std::nullopt;
  }

  transition on_message(MessageType type, buffer_ptr payload) override {
    switch (type) {
      case MessageType::kResLogout:
        return handle_res_logout(*payload);
      case MessageType::kFwdClient:
        return handle_fwd_client(*payload);
      case MessageType::kNtfLeaveRoom:
        return handle_ntf_leave_room(*payload);
      case MessageType::kResLeaveRoom:
        return handle_res_leave_room(*payload);
      default:
        return base_state::on_message(type, std::move(payload));
    }
  }

 private:
  enum class pending_input { kCommand, kMessage, kKickoutUser };

  explicit state_in_room(context* context, message_sender sender)
      : base_state(context, std::move(sender)) {
    state_ = State::kInRoom;
  }

  void show_command_prompt() {
    show_prompt(
        "possible command \n1) logout 2) send message 3) leave room "
        "4) kickout");
  }

  transition handle_res_logout(const raw_buffer& payload) {
    ResLogout logout;
    if (!rsp::libs::message::serializer::deserialize(payload, &logout)) {
      logger_.error() << "failed to parse logout" << lg::L_endl;
      return std::nullopt;
    }

    logger_.info() << "success to logout, bye bye:" << logout.uid()
                   << lg::L_endl;
    return State::kExit;
  }

  transition handle_fwd_client(const raw_buffer& payload) {
    FwdClient forward;
    if (!rsp::libs::message::serializer::deserialize(payload, &forward)) {
      logger_.error() << "failed to fwd room" << lg::L_endl;
      return std::nullopt;
    }

    std::string message;
    switch (forward.sender_type()) {
      case SenderType::kContent:
        message = "room sent message: " + forward.message();
        break;
      case SenderType::kUser:
        message =
            std::format("({0}):{1}", forward.sender_uid(), forward.message());
        break;
      default:
        logger_.warn() << "room message was sent but unknown sender type"
                       << lg::L_endl;
        return std::nullopt;
    }
    std::cout << "\x1b[" << color::kBlue << "m" << message << "\x1b[0m\n"
              << "command > " << std::flush;
    return std::nullopt;
  }

  transition handle_ntf_leave_room(const raw_buffer& payload) {
    NtfLeaveRoom notification;
    if (!rsp::libs::message::serializer::deserialize(payload, &notification)) {
      logger_.error() << "failed to deserialize ntf leave room" << lg::L_endl;
      return std::nullopt;
    }

    logger_.info() << "ntf_leave_room received, reason:"
                   << notification.reason()
                   << ", kickout reason:" << notification.kickoutreason()
                   << lg::L_endl;
    context_->room_id = 0;
    return State::kLoggedIn;
  }

  transition handle_res_leave_room(const raw_buffer& payload) {
    ResLeaveRoom response;
    if (!rsp::libs::message::serializer::deserialize(payload, &response)) {
      logger_.error() << "failed to deserialize res leave room" << lg::L_endl;
      return std::nullopt;
    }

    logger_.info() << "res_leave_room received: success?: "
                   << response.success() << lg::L_endl;
    if (!response.success()) {
      show_prompt("unable to leave room");
      return std::nullopt;
    }

    context_->room_id = 0;
    return State::kLoggedIn;
  }

  pending_input pending_input_ = pending_input::kCommand;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
