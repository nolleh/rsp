/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "proto/common/message_type.pb.h"
#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/serializer.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace ph = std::placeholders;
namespace message = rsp::libs::message;

using dispatcher_interface = libs::message::message_dispatcher_interface;
using lib_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
using link = rsp::libs::link::link;

#define REG_HANDLER(dispatcher, type, handler) \
  dispatcher.register_handler(                 \
      type,                                    \
      std::bind(&unicast_message_dispatcher::handler, this, ph::_1, ph::_2))

template <typename Handler>
class unicast_message_dispatcher : public dispatcher_interface {
 public:
  explicit unicast_message_dispatcher(Handler* handler)
      : dispatcher_(lib_dispatcher::instance()), handler_(handler) {
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomResFwdClient,
                handle_buffer<User2RoomResFwdClient>);
    dispatcher_.register_unknown_message_handler(
        std::bind(&unicast_message_dispatcher::handle_unknown, this, ph::_1));
  }

  void dispatch(MessageType type, const raw_buffer& buffer,
                link* link) override {
    dispatcher_.dispatch(type, buffer, link);
  }

  void handle_unknown(link* l) {
    auto& logger = lg::logger();
    logger.warn() << "received unknown message." << lg::L_endl;
  }

  template <typename T>
  void handle_buffer(buffer_ptr buffer, link* l) {
    T t;
    message::serializer::deserialize(*buffer, &t);
    handler_->on_recv(t);
  }

 private:
  Handler* handler_;
  lib_dispatcher& dispatcher_;
};
}  // namespace room
}  // namespace rsp
