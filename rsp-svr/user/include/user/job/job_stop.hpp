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
  explicit job_stop(const session_ptr& session, const bool force_close)
      : session_(session), force_close_(force_close) {}

  void run() {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_stop" << lg::L_endl;
    stop(session_);
  }

  void stop(const session_ptr& session) {
    session->stop(force_close_);
    session_manager::instance().remove_session(*session);
  }

  const session_ptr session_;
  const bool force_close_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
