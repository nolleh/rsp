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
  auto stop = std::make_shared<job::job_stop>(shared_from_this());
  enqueue_job(stop);
}

template <>
void session::on_recv(Ping& ping) {
  last_received_ = std::time(nullptr);
  Pong pong;
  const auto buffer =
      rsp::libs::message::serializer::serialize(MessageType::kPong, pong);
  send(buffer);
}
template <>
void session::on_recv(ReqLogin& req_login) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto login = std::make_shared<job::job_login>(shared_from_this(), req_login);
  enqueue_job(login);
}

template <>
void session::on_recv(ReqLogout& request) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto logout = std::make_shared<job::job_logout>(shared_from_this(), request);
  enqueue_job(logout);
}

}  // namespace session
}  // namespace user
}  // namespace rsp
