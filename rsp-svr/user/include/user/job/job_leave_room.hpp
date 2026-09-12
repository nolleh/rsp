/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
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

class job_leave_room : public job,
                      public std::enable_shared_from_this<job_leave_room> {
 public:
  explicit job_leave_room(const session_ptr& session,
                         const ReqLeaveRoom& leave_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(leave_room) {}

  void run() {
    lg::logger().debug() << "job_join_room: " << request_.DebugString()
                         << lg::L_endl;

    User2RoomReqLeaveRoom request;
    request.set_uid(session_->uid());

    intranet_.room().send_request(
        MessageType::kUser2RoomReqLeaveRoom, request,
        std::bind(&job_leave_room::handle_res_leave_room, shared_from_this(),
                  ph::_1));
  }

  void handle_res_leave_room(const std::shared_ptr<Message> msg) {
    auto room_response = std::dynamic_pointer_cast<User2RoomResLeaveRoom>(msg);
    lg::logger().trace() << "handle_res_leave_room: "
                         << room_response->DebugString() << lg::L_endl;
    session_->set_leave_room();

    ResLeaveRoom response;
    response.set_request_id(request_.request_id());
    response.set_success(room_response->success());

    const auto buffer =
        message::serializer::serialize(MessageType::kResLeaveRoom, response);
    session_->send(buffer);
  }

 private:
  const intranet& intranet_;
  const ReqLeaveRoom request_;
  const session_ptr session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
