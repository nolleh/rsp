/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <charconv>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_login : public base_state {
 public:
  static std::unique_ptr<base_state> create(context* context,
                                            message_sender sender) {
    return std::unique_ptr<state_login>(
        new state_login(context, std::move(sender)));
  }

  void enter() override {
    awaiting_room_id_ = false;
    show_prompt("possible command \n1) logout, 2) create_room, 3) join_room");
  }

  transition on_command(std::string_view command) override {
    if (awaiting_room_id_) {
      uint64_t room_id = 0;
      const auto [end, error] = std::from_chars(
          command.data(), command.data() + command.size(), room_id);
      if (error != std::errc() || end != command.data() + command.size()) {
        show_prompt("room id must be a number");
        return std::nullopt;
      }

      awaiting_room_id_ = false;
      ReqJoinRoom join_room;
      join_room.set_room_id(room_id);
      join_room.set_request_id(get_request_id());
      send_message(MessageType::kReqJoinRoom, join_room);
      return std::nullopt;
    }

    if (command == "1") {
      ReqLogout logout;
      logout.set_request_id(get_request_id());
      send_message(MessageType::kReqLogout, logout);
    } else if (command == "2") {
      ReqCreateRoom create_room;
      create_room.set_request_id(get_request_id());
      send_message(MessageType::kReqCreateRoom, create_room);
    } else if (command == "3") {
      awaiting_room_id_ = true;
      show_prompt("put room id that you want to enter");
    } else {
      show_prompt("your command is incorrect");
    }
    return std::nullopt;
  }

  transition on_message(MessageType type, buffer_ptr payload) override {
    switch (type) {
      case MessageType::kResLogout:
        return handle_res_logout(*payload);
      case MessageType::kResCreateRoom:
        return handle_res_create_room(*payload);
      case MessageType::kResJoinRoom:
        return handle_res_join_room(*payload);
      default:
        return base_state::on_message(type, std::move(payload));
    }
  }

 private:
  explicit state_login(context* context, message_sender sender)
      : base_state(context, std::move(sender)) {
    state_ = State::kLoggedIn;
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

  transition handle_res_create_room(const raw_buffer& payload) {
    ResCreateRoom create_room;
    if (!rsp::libs::message::serializer::deserialize(payload, &create_room)) {
      logger_.error() << "failed to parse created room" << lg::L_endl;
      return std::nullopt;
    }

    context_->room_id = create_room.room_id();
    logger_.info() << "created room #" << context_->room_id << ", and joined"
                   << lg::L_endl;
    return State::kInRoom;
  }

  transition handle_res_join_room(const raw_buffer& payload) {
    ResJoinRoom join_room;
    if (!rsp::libs::message::serializer::deserialize(payload, &join_room)) {
      logger_.error() << "failed to parse join room" << lg::L_endl;
      return std::nullopt;
    }

    if (!join_room.success()) {
      logger_.info() << "unable to join room" << lg::L_endl;
      enter();
      return std::nullopt;
    }

    context_->room_id = join_room.room_id();
    logger_.info() << "joined room #" << context_->room_id << lg::L_endl;
    return State::kInRoom;
  }

  bool awaiting_room_id_ = false;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
