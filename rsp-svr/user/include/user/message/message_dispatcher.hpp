/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <istream>
#include <memory>
#include <streambuf>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"
#include "user/job/job_login.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace message {

namespace ph = std::placeholders;
namespace message = libs::message;
using dispatcher_interface = libs::message::message_dispatcher_interface;
using lib_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
using link = rsp::libs::link::link;

#define REG_HANDLER(dispatcher, type, handler) \
  dispatcher.register_handler2(                \
      type, std::bind(&message_dispatcher::handler, this, ph::_1, ph::_2))

class message_dispatcher : public dispatcher_interface {
 public:
  message_dispatcher() : dispatcher_(lib_dispatcher::instance()) {
    REG_HANDLER(dispatcher_, MessageType::kReqLogin, handle_buffer_req_login);
    REG_HANDLER(dispatcher_, MessageType::kReqLogout, handle_buffer_req_logout);
  }

  void dispatch(MessageType type, const raw_buffer& buffer,
                link* link) override {
    dispatcher_.dispatch(type, buffer, link);
  }

  // TOOD(@nolleh) hum. actually now, no need to manage the buffer as s_ptr.
  // client also need to be chagned to add some layer.
  // let's consider after development was got some where.
  void handle_buffer_req_login(buffer_ptr buffer, link* l) {
    ReqLogin req_login;
    pass_to_session(buffer, &req_login, l);
  }

  void handle_buffer_req_logout(buffer_ptr buffer, link* l) {
    ReqLogout req_logout;
    pass_to_session(buffer, &req_logout, l);
  }

  template <typename Message>
  void pass_to_session(buffer_ptr buffer, Message* message, link* l) {
    auto success = message::serializer::deserialize(*buffer, message);
    if (!success) {
      namespace lg = rsp::libs::logger;
      auto& logger = lg::logger();
      logger.error() << "something wrong. failed to parse login message"
                     << lg::L_endl;
      return;
    }

    auto session = dynamic_cast<session::session*>(l);
    session->on_recv(*message);
  }

 private:
  lib_dispatcher& dispatcher_;
};
}  // namespace message
}  // namespace user
}  // namespace rsp
