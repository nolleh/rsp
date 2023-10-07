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

class job_logout : public rsp::libs::job::job {
 public:
  void run(rsp::libs::link::link* link) {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_logout: " << request_.uid() << lg::L_endl;
    auto session = dynamic_cast<session::session*>(link);
    session::session_manager::instance().remove_session(*session);
    send_res_logout(session);
  }

  void send_res_logout(session::session* session) {
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
