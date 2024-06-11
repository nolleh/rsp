/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

// #include <format>
#include <memory>
#include <numeric>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

template <typename... Args>
inline std::string join(char delimiter, Args&&... args) {
  auto range = {args...};
  auto result = std::accumulate(
      range.begin(), range.end(), "",
      [&delimiter](auto& acc, auto& val) { return acc + delimiter + val; });
  return result;
}

class state_login : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket) {
    return std::shared_ptr<state_login>(new state_login(socket));
  }

  ~state_login() {
    dispatcher_.unregister_handler(MessageType::kResLogout);
    dispatcher_.unregister_handler(MessageType::kResCreateRoom);
    dispatcher_.unregister_handler(MessageType::kResJoinRoom);
  }

  void init() override {
    dispatcher_.register_handler(
        MessageType::kResLogout,
        std::bind(&state_login::handle_res_logout, this, std::placeholders::_1,
                  std::placeholders::_2));

    dispatcher_.register_handler(
        MessageType::kResCreateRoom,
        std::bind(&state_login::handle_res_create_room, this,
                  std::placeholders::_1, std::placeholders::_2));

    dispatcher_.register_handler(
        MessageType::kResJoinRoom,
        std::bind(&state_login::handle_res_join_room, this,
                  std::placeholders::_1, std::placeholders::_2));

    // std::string commands[]{"logout", "create_room", "join_room"};
    //
    // auto command_direction = join(',', commands);
    // prompt_ << std::format("possible command \n{}\n", command_direction);
    prompt_ << "possible command \n1) logout, 2) create_room, 3) join_room\n";
    std::cout << "> ";
    std::string command;
    std::cin >> command;

    if (command == "1") {
      send_message<ReqLogout>(MessageType::kReqLogout);
    } else if (command == "2") {
      send_message<ReqCreateRoom>(MessageType::kReqCreateRoom);
    } else if (command == "3") {
      prompt_ << "put room id that you want to enter\n";
      std::cout << "> ";
      uint64_t room_id;
      std::cin >> room_id;
      send_join_room_message(room_id);
    }
  }

 protected:
  explicit state_login(socket* socket) : base_state(socket) {
    state_ = State::kLoggedIn;
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

  void handle_res_create_room(buffer_ptr buffer, link*) {
    ResCreateRoom create_room;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &create_room)) {
      logger_.error() << "failed to parse created room" << lg::L_endl;
      return;
    }

    logger_.info() << "created room #" << create_room.room_id()
                   << ", and joined" << lg::L_endl;
    next_ = State::kInRoom;
  }

  void handle_res_join_room(buffer_ptr buffer, link*) {
    ResJoinRoom join_room;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &join_room)) {
      logger_.error() << "failed to parse join room" << lg::L_endl;
      return;
    }

    logger_.info() << "joined room #" << join_room.room_id() << lg::L_endl;
    next_ = State::kInRoom;
  }

  template <typename T>
  void send_message(MessageType type) {
    T msg;
    msg.set_request_id(get_request_id());
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

  void send_join_room_message(uint64_t room_id) {
    ReqJoinRoom join_room;
    join_room.set_room_id(room_id);
    join_room.set_request_id(get_request_id());
    auto message = rsp::libs::message::serializer::serialize(
        MessageType::kReqJoinRoom, join_room);
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
