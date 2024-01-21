/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/common/ping.pb.h"
#include "room/intranet/message_dispatcher.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"

namespace rsp {
namespace room {
namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
using link = rsp::libs::link::link;

class room_receiver {
 public:
  room_receiver()
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(room_message_handler()) {
    room_receiver_ =
        br::broker::s_create_subscriber(CastType::kAnyCast, "room", 1, "topic");
  }

  void start() {
    room_receiver_->start();
    // logger_.info() << "waiting message is ready" << lg::L_endl;
    // sleep(3);
    start_recv();
  }

  void start_recv() {
    auto buffer = room_receiver_->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    start_recv();
  }

  template <typename T>
  void on_recv(const T& msg) {
    message_handler_.handle(msg);
  }

 private:
  lg::s_logger& logger_;
  message_dispatcher<room_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> room_receiver_;
  room_message_handler message_handler_;
};

template <>
void room_receiver::on_recv(const Ping& ping) {
  logger_.debug() << "received ping" << lg::L_endl;
}

class intranet {
 public:
  intranet() : logger_(lg::logger()) {}

  void start() { room_receiver_.start(); }

 private:
  lg::s_logger& logger_;
  room_receiver room_receiver_;
};

}  // namespace room
}  // namespace rsp
