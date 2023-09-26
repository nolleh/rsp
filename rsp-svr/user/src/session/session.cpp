#include <iostream>

#include "job/job_login.hpp"
#include "session/session.hpp"

namespace rsp {
namespace user {
namespace session {

template <>
void session::on_recv(ReqLogin&& req_login) {
  std::cout << "session received req_login" << std::endl;

  rsp::user::job::job_login job{req_login};
  scheduler_.push_and_run(&job);
}

}  // namespace session
}  // namespace user
}  // namespace rsp
