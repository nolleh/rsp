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

using session = rsp::user::session::session;
using session_manager = rsp::user::session::session_manager;

class job_logout : public rsp::libs::job::job {
 public:
  explicit job_logout(const ReqLogout& logout) : request_(logout) {}

  void run(rsp::libs::link::link* link) {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_logout: " << request_.uid() << lg::L_endl;
    auto s = dynamic_cast<session*>(link);
    send_res_logout(s);
    session_manager::instance().remove_session(*s);
  }

  void send_res_logout(session* session) {
    ResLogout logout;
    auto buffer = rsp::libs::message::serializer::serialize(
        MessageType::kResLogout, logout);
    session->send(buffer);
  }

 private:
  ReqLogout request_;
};
}  // namespace job
}  // namespace user
}  // namespace rsp
