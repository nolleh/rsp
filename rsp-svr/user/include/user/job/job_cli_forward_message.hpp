
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "proto/user/to_client.pb.h"
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

class job_cli_forward_message
    : public job,
      public std::enable_shared_from_this<job_cli_forward_message> {
 public:
  explicit job_cli_forward_message(const session_ptr& session,
                                   const User2RoomReqFwdClient& fwd_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(fwd_room) {}

  void run() {
    lg::logger().debug() << "job_cli_forward_message: "
                         << request_.DebugString() << lg::L_endl;

    ReqFwdClient request;
    // TODO(@nolleh) need to be changed
    request.set_request_id(request_.request_id());
    request.set_uid(request_.uid());
    request.set_sender_type(request_.sender_type());
    request.set_sender_uid(request_.sender_uid());
    request.set_message(request_.message());
    const auto buffer =
        message::serializer::serialize(MessageType::kReqFwdClient, request);
    session_->send(buffer);

    // TODO(@nolleh) send response to room server
  }

 private:
  const intranet& intranet_;
  const session_ptr session_;
  const User2RoomReqFwdClient request_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
