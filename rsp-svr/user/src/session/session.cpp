/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "user/job/job_login.hpp"
#include "user/job/job_logout.hpp"
#include "user/job/job_stop.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace session {

void session::enqueue_stop() {
  auto stop = std::make_shared<job::job_stop>();
  enqueue_job(stop);
}

template <>
void session::on_recv(ReqLogin& req_login) {
  namespace job = rsp::user::job;
  auto login = std::make_shared<job::job_login>(req_login);
  enqueue_job(login);
}

template <>
void session::on_recv(ReqLogout& request) {
  namespace job = rsp::user::job;
  auto logout = std::make_shared<job::job_logout>(request);
  enqueue_job(logout);
}

}  // namespace session
}  // namespace user
}  // namespace rsp
