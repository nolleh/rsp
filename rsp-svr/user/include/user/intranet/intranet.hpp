/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <typeinfo>

// #include <google/protobuf/port_def.inc>

#include "rsplib/logger/logger.hpp"
#include "user/intranet/room_channel.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace libs = rsp::libs;

class intranet {
 public:
  static intranet& instance();

  void start() { room_channel_.start(); }

  void stop() { room_channel_.stop(); }

  room_channel& room() const { return room_channel_; }

 private:
  intranet(const intranet&) = delete;
  intranet& operator=(const intranet&) = delete;

  static std::once_flag s_flag;
  static std::unique_ptr<intranet> s_instance;

  intranet() : logger_(lg::logger()) {}

  lg::s_logger& logger_;
  mutable room_channel room_channel_;
};

}  // namespace user
}  // namespace rsp
