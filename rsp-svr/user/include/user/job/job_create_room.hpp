
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
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
namespace lg = rsp::libs::logger;

using job = rsp::libs::job::job;
using link = rsp::libs::link::link;
using session_ptr = rsp::user::session::session_ptr;

class job_create_room : public job {
 public:
  explicit job_create_room(const session_ptr& session,
                           const ReqCreateRoom& create_room)
      : session_(session), request_(create_room) {}

  void run() {
    lg::logger().debug() << "job_create_room: " << request_.request_id()
                         << lg::L_endl;
  }

 private:
  const ReqCreateRoom request_;
  const session_ptr session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
