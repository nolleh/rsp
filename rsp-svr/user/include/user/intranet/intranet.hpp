/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <typeinfo>

// #include <google/protobuf/port_def.inc>

#include "rsplib/logger/logger.hpp"
#include "user/intranet/room_sender.hpp"
#include "user/intranet/user_receiver.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace libs = rsp::libs;

class intranet {
 public:
  static intranet& instance();

  void start() {
    room_sender_.start();
    user_receiver_.start();
  }

  void stop() {
    user_receiver_.stop();
    room_sender_.stop();
  }

  user_receiver& me() const { return user_receiver_; }
  room_sender& room() const { return room_sender_; }

 private:
  intranet(const intranet&) = delete;
  intranet& operator=(const intranet&) = delete;

  static std::once_flag s_flag;
  static std::unique_ptr<intranet> s_instance;

  intranet() : logger_(lg::logger()) {}

  lg::s_logger& logger_;
  mutable user_receiver user_receiver_;
  mutable room_sender room_sender_;
};

}  // namespace user
}  // namespace rsp
