
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/optional.hpp>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_client.pb.h"
#include "proto/user/to_room.pb.h"
#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_in_room : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket,
                                            struct context* context) {
    return std::shared_ptr<state_in_room>(new state_in_room(socket, context));
  }

  ~state_in_room() {
    stop_ = true;
    // reader_->join();
    dispatcher_.unregister_handler(MessageType::kResLogout);
  }

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
    //
    prompt_ << "possible command \n1) logout 2) send message 3) kickout";
    read_input();
  }

 protected:
  explicit state_in_room(socket* socket, struct context* context)
      : base_state(socket, context) {
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
    dispatcher_.register_handler(
        MessageType::kReqFwdClient,
        std::bind(&state_in_room::handle_req_fwd_cli, this,
                  std::placeholders::_1, std::placeholders::_2));
    dispatcher_.register_handler(
        MessageType::kNtfLeaveRoom,
        std::bind(&state_in_room::handle_ntf_leave_room, this,
                  std::placeholders::_1, std::placeholders::_2));
    start_fwd_read();
  }

 private:
  void read_input() {
    using namespace std::chrono_literals;  // NOLINT
    std::this_thread::sleep_for(10ms);
    std::cout << "command > ";
    std::string command;
    std::cin >> command;

    try {
      switch (std::stoi(command)) {
        case 1:
          send_message(MessageType::kReqLogout, ReqLogout{});
          break;
        // case 2:
        //   std::cout << "read message.." << std::endl;
        //   // read_with_timeout(socket_, boost::posix_time::seconds(3));
        //   read_message();
        //   std::this_thread::sleep_for(std::chrono::seconds(1));
        //   init();
        //   break;
        case 2: {
          std::cout << "type message to send" << std::endl;
          std::cout << "> ";
          std::string msg;
          std::getline(std::cin >> std::ws, msg);
          ReqFwdRoom fwd;
          fwd.set_message(msg);
          send_message(MessageType::kReqFwdRoom, fwd);
          read_input();
          break;
        }
        case 3: {
          std::cout << "type user name to kickout" << std::endl;
          std::cout << "> ";
          std::string user;
          std::getline(std::cin >> std::ws, user);
          ReqFwdRoom fwd;
          // TODO(@nolleh) content message definition
          std::string kickout = "kickout:" + user;
          fwd.set_message(kickout);
          send_message(MessageType::kReqFwdRoom, fwd);
          const auto context = get_context();
          if (user != context->uid) read_input();
          break;
        }
      }
    } catch (std::invalid_argument const& ex) {
      prompt_ << "your command is incorrect";
      read_input();
    }
  }

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
    read_input();
  }

  void handle_req_fwd_cli(buffer_ptr buffer, link*) {
    ReqFwdClient fwd_client;
    if (!rsp::libs::message::serializer::deserialize(*buffer, &fwd_client)) {
      logger_.error() << "failed to fwd room" << lg::L_endl;
      return;
    }

    std::string message;
    switch (fwd_client.sender_type()) {
      case SenderType::kContent:
        message = "room sent message: " + fwd_client.message();
        break;
      case SenderType::kUser:
        message = std::format("({0}):{1}", fwd_client.sender_uid(),
                              fwd_client.message());
        break;
      default:
        logger_.warn() << "room message was sent but unknown sender type"
                       << lg::L_endl;
        return;
        break;
    }
    std::cout << "\x1b[" << color::kBlue << "m" << message << "\x1b[0m"
              << std::endl;
  }

  void handle_ntf_leave_room(buffer_ptr buffer, link*) {
    NtfLeaveRoom ntf_leave_room;
    if (!rsp::libs::message::serializer::deserialize(*buffer,
                                                     &ntf_leave_room)) {
      logger_.error() << "failed to deserialize ntf leave room" << lg::L_endl;
      return;
    }

    const auto reason = ntf_leave_room.reason();
    const auto kickout_reason = ntf_leave_room.kickoutreason();

    logger_.info() << "ntf_leave_room received, reason:" << reason
                   << ", kickout reason:" << kickout_reason << lg::L_endl;

    // sleep(1);

    std::cin.clear();
    stop_ = true;
    socket_->cancel();
    next_ = State::kLoggedIn;
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

  void start_fwd_read() {
    reader_ = std::make_unique<std::thread>([&]() {
      while (!stop_) read_message();
    });
    reader_->detach();
  }

  void read_message() {
    std::array<char, 128> buf;
    boost::system::error_code error;
    // TODO(@nolleh) check escaped after cancel
    auto len = socket_->read_some(boost::asio::buffer(buf), error);
    if (error) {
      if (boost::asio::error::eof == error) {
        return;
      }
      throw boost::system::system_error(error);
    }
    handle_buffer(buf, len);
  }

  std::unique_ptr<std::thread> reader_;
  std::atomic<bool> stop_;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
