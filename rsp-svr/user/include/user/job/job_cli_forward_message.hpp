
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

class job_cli_forward_message
    : public job,
      public std::enable_shared_from_this<job_cli_forward_message> {
 public:
  explicit job_cli_forward_message(const session_ptr& session,
                                   const User2RoomFwdClient& fwd_room)
      : intranet_(intranet::instance()),
        session_(session),
        message_(fwd_room) {}

  void run() {
    lg::logger().debug() << "job_cli_forward_message: "
                         << message_.DebugString() << lg::L_endl;

    FwdClient forward;
    forward.set_uid(message_.uid());
    forward.set_sender_type(message_.sender_type());
    forward.set_sender_uid(message_.sender_uid());
    forward.set_message(message_.message());
    const auto buffer =
        message::serializer::serialize(MessageType::kFwdClient, forward);
    session_->send(buffer);
  }

 private:
  const intranet& intranet_;
  const session_ptr session_;
  const User2RoomFwdClient message_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
