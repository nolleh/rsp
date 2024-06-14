
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rsplib/job/job.hpp"
#include "user/intranet/intranet.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
namespace lg = rsp::libs::logger;

using job = rsp::libs::job::job;
using link = rsp::libs::link::link;
using session_ptr = rsp::user::session::session_ptr;

class job_forward_message
    : public job,
      public std::enable_shared_from_this<job_forward_message> {
 public:
  explicit job_forward_message(const session_ptr& session,
                               const ReqFwdRoom& fwd_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(fwd_room) {}

  void run() {
    lg::logger().debug() << "job_forward_message: " << request_.DebugString()
                         << lg::L_endl;

    User2RoomReqFwdRoom request;
    // TODO(@nolleh) need to be changed
    request.set_request_id(request_.request_id());
    request.set_uid(request_.uid());
    request.set_message(request_.message());

    intranet_.room().send_request(
        MessageType::kUser2RoomReqFwdRoom, request,
        std::bind(&job_forward_message::handle_res_forward_message,
                  shared_from_this(), ph::_1));
  }

  void handle_res_forward_message(const std::shared_ptr<Message> msg) {
    auto room_response = std::dynamic_pointer_cast<User2RoomResFwdRoom>(msg);
    lg::logger().trace() << "handle_res_fwd_room: success:"
                         << room_response->success() << lg::L_endl;

    ResCreateRoom response;
    response.set_request_id(room_response->request_id());
    response.set_success(room_response->success());

    const auto buffer =
        message::serializer::serialize(MessageType::kResFwdRoom, response);
    session_->send(buffer);
  }

 private:
  const intranet& intranet_;
  const session_ptr session_;
  const ReqFwdRoom request_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
