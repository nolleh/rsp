/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/common/ping.pb.h"
#include "room/intranet/user_channel.hpp"
#include "rsplib/link/link.hpp"

namespace rsp {
namespace room {
namespace lg = rsp::libs::logger;
using link = rsp::libs::link::link;

class intranet {
 public:
  static intranet& instance();

  ~intranet() {
    stop();
    // Room callbacks may still use this channel during contents teardown.
    // Drain them while the channel object and its shared library are alive.
    room_manager::instance().shutdown();
  }

  // TODO(@nolleh) check
  user_channel& channel() { return user_channel_; }

  void start() { user_channel_.start(); }
  void stop() { user_channel_.stop(); }

 private:
  intranet() : logger_(lg::logger()), user_channel_() {}

  intranet(const intranet&) = delete;
  intranet& operator=(const intranet&) = delete;


  lg::s_logger& logger_;
  user_channel user_channel_;
};

}  // namespace room
}  // namespace rsp
