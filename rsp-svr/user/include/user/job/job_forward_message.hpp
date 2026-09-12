
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

class job_forward_message : public job {
 public:
  explicit job_forward_message(const session_ptr& session,
                               const FwdRoom& fwd_room)
      : intranet_(intranet::instance()),
        session_(session),
        message_(fwd_room) {}

  void run() {
    lg::logger().debug() << "job_forward_message: " << message_.DebugString()
                         << lg::L_endl;

    User2RoomFwdRoom message;
    message.set_uid(session_->uid());
    message.set_message(message_.message());

    intranet_.room().send_notification(MessageType::kUser2RoomFwdRoom, message);
  }

 private:
  const intranet& intranet_;
  const session_ptr session_;
  const FwdRoom message_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
