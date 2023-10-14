/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include "proto/user/login.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "user/session/session.hpp"
#include "user/session/session_manager.hpp"

namespace rsp {
namespace user {
namespace job {

using session_ptr = rsp::user::session::session_ptr;
using session_manager = rsp::user::session::session_manager;

class job_logout : public rsp::libs::job::job {
 public:
  explicit job_logout(const session_ptr session, const ReqLogout& logout)
      : session_(session), request_(logout) {}

  void run() {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_logout: " << session_->uid() << lg::L_endl;
    send_res_logout(session_);
    session_->enqueue_stop(false);
  }

  void send_res_logout(const session_ptr& session) {
    ResLogout logout;
    logout.set_uid(session->uid());
    auto buffer = rsp::libs::message::serializer::serialize(
        MessageType::kResLogout, logout);
    session->send(buffer);
  }

 private:
  const ReqLogout request_;
  const session_ptr session_;
};
}  // namespace job
}  // namespace user
}  // namespace rsp
