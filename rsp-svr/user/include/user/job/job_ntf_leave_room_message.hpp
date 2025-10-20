
/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_client.pb.h"
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

class job_ntf_leave_room_message
    : public job,
      public std::enable_shared_from_this<job_ntf_leave_room_message> {
 public:
  explicit job_ntf_leave_room_message(const session_ptr& session,
                                      const User2RoomNtfLeaveRoom& notification)
      : intranet_(intranet::instance()),
        session_(session),
        notification_(notification) {}

  void run() {
    lg::logger().debug() << "ntf_leave_room message received: "
                         << notification_.DebugString() << lg::L_endl;

    NtfLeaveRoom ntf_leave_room;
    ntf_leave_room.set_reason(notification_.reason());
    const auto buffer = message::serializer::serialize(
        MessageType::kNtfLeaveRoom, ntf_leave_room);
    session_->send(buffer);
    session_->set_leave_room();
  }

 private:
  const intranet& intranet_;
  const session_ptr session_;
  const User2RoomNtfLeaveRoom notification_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
