/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "user/job/job_login.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace session {

template <>
void session::on_recv(ReqLogin& req_login) {
  namespace job = rsp::user::job;
  auto login = std::make_shared<job::job_login>(req_login);
  // current design is no need worry for session life. (need to consider)
  scheduler_.push_and_run(login, this);
}

}  // namespace session
}  // namespace user
}  // namespace rsp
