/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <typeinfo>

#include "proto/common/message_type.pb.h"
#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"
#include "proto/user/to_client.pb.h"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/serializer.hpp"
#include "user/session/session_manager.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace ph = std::placeholders;

using dispatcher_interface = libs::message::message_dispatcher_interface;
using lib_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
using link = rsp::libs::link::link;

#define REG_HANDLER(dispatcher, type, handler) \
  dispatcher.register_handler(                 \
      type, std::bind(&message_dispatcher::handler, this, ph::_1, ph::_2))


template <typename Handler>
class message_dispatcher : public dispatcher_interface {
 public:
  explicit message_dispatcher(Handler* handler)
      : dispatcher_(lib_dispatcher::instance()), handler_(handler) {
    // TODO(@nolleh)
    REG_HANDLER(dispatcher_, MessageType::kPing, handle_buffer<Ping>);
    REG_HANDLER(dispatcher_, MessageType::kPong, handle_buffer<Pong>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomResCreateRoom,
                handle_buffer<User2RoomResCreateRoom>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomResJoinRoom,
                handle_buffer<User2RoomResJoinRoom>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomResFwdRoom,
                handle_buffer<User2RoomResFwdRoom>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomReqFwdClient,
                handle_buffer<User2RoomReqFwdClient>);
    dispatcher_.register_unknown_message_handler(
        std::bind(&message_dispatcher::handle_unknown, this, ph::_1));
  }

#undef REG_HANDLER
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
    auto& logger = lg::logger();
    T t;
    logger.debug() << "dispatched. deserialize and invoke handler, type:"
                   << typeid(t).name() << lg::L_endl;
    libs::message::serializer::deserialize(*buffer, &t);
    handler_->on_recv(t);
  }

 private:
  Handler* handler_;
  lib_dispatcher& dispatcher_;
};

}  // namespace user
}  // namespace rsp
