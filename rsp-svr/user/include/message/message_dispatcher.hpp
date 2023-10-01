#pragma once
#include <istream>
#include <memory>
#include <streambuf>

#include "job/job_login.hpp"
#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/message/imemstream.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/types.hpp"
#include "session/session.hpp"

namespace rsp {
namespace user {
namespace message {

namespace ph = std::placeholders;
using dispatcher_interface = libs::message::message_dispatcher_interface;
using lib_dispatcher = libs::message::message_dispatcher;
using raw_buffer = libs::message::raw_buffer;
using buffer_ptr = libs::message::buffer_ptr;
using job_scheduler = libs::job::job_scheduler;
using link = rsp::libs::link::link;
using handler2 = rsp::libs::message::handler2;

class message_dispatcher : public dispatcher_interface {
 public:
  message_dispatcher() : dispatcher_(lib_dispatcher::instance()) {
    // TODO(@nolleh) looks like it is better change this logic to macro
    dispatcher_.register_handler2(
        MessageType::REQ_LOGIN,
        std::bind(&message_dispatcher::handle_buffer_req_login, this, ph::_1,
                  ph::_2));
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
    imemstream stream(const_cast<const char*>(buffer->data()), buffer->size());
    auto success = req_login.ParseFromIstream(&stream);
    if (!success) {
      std::cout << "something wrong. failed to parse login message"
                << std::endl;
      return;
    }
    // TODO(@nolleh) workerthread... post
    auto session = dynamic_cast<session::session*>(l);
    session->on_recv(req_login);
    // job::job_login job{req_login};
    //
    // scheduler_->push_and_run(&job, std::shared_ptr<link>(l));
  }

 private:
  lib_dispatcher& dispatcher_;
  // job_scheduler* scheduler_;
};
}  // namespace message
}  // namespace user
}  // namespace rsp