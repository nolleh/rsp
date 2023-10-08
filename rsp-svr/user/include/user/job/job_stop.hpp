/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include "user/session/session.hpp"
#include "user/session/session_manager.hpp"
namespace rsp {
namespace user {
namespace job {

using session = rsp::user::session::session;
using session_manager = rsp::user::session::session_manager;

/**
 * has role to final clean up session.
 */
class job_stop : public rsp::libs::job::job {
 public:
  void run(rsp::libs::link::link* link) {
    namespace lg = rsp::libs::logger;
    lg::logger().debug() << "job_stop" << lg::L_endl;
    auto s = dynamic_cast<session*>(link);
    stop(s);
  }

  void stop(session* session) {
    session->stop();
    session_manager::instance().remove_session(*session);
  }
};

}  // namespace job
}  // namespace user
}  // namespace rsp
