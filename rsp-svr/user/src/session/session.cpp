/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "user/session/session.hpp"

#include <iostream>

#include "user/job/job_cli_forward_message.hpp"
#include "user/job/job_create_room.hpp"
#include "user/job/job_forward_message.hpp"
#include "user/job/job_join_room.hpp"
#include "user/job/job_login.hpp"
#include "user/job/job_logout.hpp"
#include "user/job/job_stop.hpp"

namespace rsp {
namespace user {
namespace session {

void session::enqueue_stop(bool force) {
  auto stop = std::make_shared<job::job_stop>(shared_from_this(), force);
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

template <>
void session::on_recv(ReqCreateRoom& request) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto runner =
      std::make_shared<job::job_create_room>(shared_from_this(), request);
  enqueue_job(runner);
}

template <>
void session::on_recv(ReqJoinRoom& request) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto runner =
      std::make_shared<job::job_join_room>(shared_from_this(), request);
  enqueue_job(runner);
}

template <>
void session::on_recv(ReqFwdRoom& request) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto runner =
      std::make_shared<job::job_forward_message>(shared_from_this(), request);
  enqueue_job(runner);
}

template <>
void session::on_recv(const User2RoomReqFwdClient& request) {
  last_received_ = std::time(nullptr);
  namespace job = rsp::user::job;
  auto runner = std::make_shared<job::job_cli_forward_message>(
      shared_from_this(), request);
  enqueue_job(runner);
}

}  // namespace session
}  // namespace user
}  // namespace rsp
