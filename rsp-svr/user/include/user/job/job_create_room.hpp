
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "user/intranet/intranet.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
namespace lg = rsp::libs::logger;
namespace ph = std::placeholders;

using job = rsp::libs::job::job;
using session_ptr = rsp::user::session::session_ptr;

class job_create_room : public job,
                        public std::enable_shared_from_this<job_create_room> {
 public:
  explicit job_create_room(const session_ptr& session,
                           const ReqCreateRoom& create_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(create_room) {}

  void run() {
    lg::logger().debug() << "job_create_room: " << request_.request_id()
                         << lg::L_endl;
    intranet_.room().send_request(
        MessageType::kReqCreateRoom, request_,
        std::bind(&job_create_room::handle_res_create_room, shared_from_this(),
                  ph::_1));
  }

  void handle_res_create_room(const std::shared_ptr<Message> msg) {
    auto response = std::dynamic_pointer_cast<ResCreateRoom>(msg);
    session_->set_enter_room(response->room_id());

    const auto buffer =
        message::serializer::serialize(MessageType::kResCreateRoom, *response);
    session_->send(buffer);
  }

 private:
  const intranet& intranet_;
  const ReqCreateRoom request_;
  const session_ptr session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
