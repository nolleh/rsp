
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
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

class job_create_room : public job,
                        public std::enable_shared_from_this<job_create_room> {
 public:
  explicit job_create_room(const session_ptr& session,
                           const ReqCreateRoom& create_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(create_room) {}

  void run() {
    lg::logger().debug() << "job_create_room: " << request_.DebugString()
                         << lg::L_endl;

    User2RoomReqCreateRoom request;
    // TODO(@nolleh) need to be changed
    request.set_request_id(request_.request_id());
    request.set_uid(session_->uid());
    request.set_addr(intranet_.me().addr());

    intranet_.room().send_request(
        MessageType::kUser2RoomReqCreateRoom, request,
        std::bind(&job_create_room::handle_res_create_room, shared_from_this(),
                  ph::_1));
  }

  void handle_res_create_room(const std::shared_ptr<Message> msg) {
    auto room_response = std::dynamic_pointer_cast<User2RoomResCreateRoom>(msg);
    lg::logger().trace() << "handle_res_create_room: room_id:"
                         << room_response->room_id() << lg::L_endl;
    session_->set_enter_room(room_response->room_id());

    ResCreateRoom response;
    response.set_request_id(room_response->request_id());
    response.set_room_id(room_response->room_id());
    response.set_success(room_response->success());

    const auto buffer =
        message::serializer::serialize(MessageType::kResCreateRoom, response);
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
