/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include "user/session/session.hpp"
#include "user/session/session_manager.hpp"
namespace rsp {
namespace user {
namespace job {

using session = rsp::user::session::session;
using session_manager = rsp::user::session::session_manager;

using session_ptr = rsp::user::session::session_ptr;
/**
 * has role to final clean up session.
 */
class job_stop : public rsp::libs::job::job {
 public:
  explicit job_stop(const session_ptr& session):session_(session) {}
  void run() {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_stop" << lg::L_endl;
    stop(session_);
  }

  void stop(const session_ptr& session) {
    session->stop();
    session_manager::instance().remove_session(*session);
  }

  session_ptr const session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
