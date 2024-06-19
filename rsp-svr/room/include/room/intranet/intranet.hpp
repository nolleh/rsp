/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/common/ping.pb.h"
#include "rsplib/link/link.hpp"
#include "room/intranet/room_receiver.hpp"

namespace rsp {
namespace room {
namespace lg = rsp::libs::logger;
using link = rsp::libs::link::link;


class intranet {
 public:
  static intranet& instance();
  intranet() : logger_(lg::logger()), room_receiver_(this) {}

  // TODO(@nolleh) check
  const room_receiver& responder() const { return room_receiver_; }

  void start() { room_receiver_.start(); }

 private:
  static std::once_flag s_flag;
  static std::unique_ptr<intranet> s_instance;

  intranet(const intranet&) = delete;
  intranet& operator=(const intranet&) = delete;

  lg::s_logger& logger_;
  room_receiver room_receiver_;
};

}  // namespace room
}  // namespace rsp
