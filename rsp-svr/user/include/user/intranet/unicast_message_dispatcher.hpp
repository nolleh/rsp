/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "proto/common/message_type.pb.h"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "user/session/session.hpp"

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
      type,                                    \
      std::bind(&unicast_message_dispatcher::handler, this, ph::_1, ph::_2))

template <typename Handler>
class unicast_message_dispatcher : public dispatcher_interface {
 public:
  explicit unicast_message_dispatcher(Handler* handler)
      : dispatcher_(lib_dispatcher::instance()), handler_(handler) {
    // REG_HANDLER(dispatcher_, MessageType::kPing, handle_buffer<Ping>);
    // REG_HANDLER(dispatcher_, MessageType::kPong, handle_buffer<Pong>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomReqFwdClient,
                handle_buffer<User2RoomReqFwdClient>);
    REG_HANDLER(dispatcher_, MessageType::kUser2RoomNtfLeaveRoom,
                handle_buffer<User2RoomNtfLeaveRoom>);
    dispatcher_.register_unknown_message_handler(
        std::bind(&unicast_message_dispatcher::handle_unknown, this, ph::_1));
  }
#undef REG_HANDLER
  void dispatch(MessageType type, const raw_buffer& buffer,
                link* link) override {
    dispatcher_.dispatch(type, buffer, link);
  }

  void handle_unknown(link* l) {
    auto& logger = lg::logger();
    logger.warn() << "received unknown message." << lg::L_endl;
    auto session = dynamic_cast<session::session*>(l);
    session->on_recv_unknown();
  }

  // TODO(@nolleh) hum. actually now, no need to manage the buffer as s_ptr.
  // client also need to be changed to add some layer.
  // let's consider after development was got some where.
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
