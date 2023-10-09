/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
// logged in users
#include <memory>
#include <string>

#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_connection.hpp"
#include "user/server/worker.hpp"

namespace rsp {
namespace user {
namespace session {

enum class UserState {
  kLogouted,
  kLoggedIn,
  kDetached,
  kInRoom,
};

using link = libs::link::link;
namespace server = libs::server;
namespace lg = rsp::libs::logger;
using job_scheduler = libs::job::job_scheduler;
using worker = rsp::user::server::worker;

class session;
using session_ptr = std::shared_ptr<session>;

class session : public link, public std::enable_shared_from_this<session> {
 public:
  explicit session(server::connection_ptr conn)
      : worker_(worker::instance()), link(conn), state_(UserState::kLogouted) {
    session* self = this;
    conn->attach_link(self);
  }

  ~session() {
    // session destroy meaning logout
  }

  void on_connected() override {
    // TODO(@nolleh) notify to other servers that user attached
  }

  // unexpected discon
  void on_disconnected() override {
    // TODO(@nolleh) notify to other servers that user detached
  }

  // by client/server, requested closed was done
  void on_closed() override {
    // nothing todo
  }

  template <typename Message>
  void on_recv(Message&& msg) {
    lg::logger().error() << "session - on_recv, unknown message" << lg::L_endl;
    // throw std::exception();
  }

  void enqueue_stop();

  void set_user(const std::string& uid) { uid_ = uid; }

  std::string uid() { return uid_; }

  uint16_t room_id() { return room_id_; }

 private:
  void enqueue_job(libs::job::job_ptr job) {
    auto runner = worker_.wrap(std::bind(&libs::job::job::run, job, this));
    scheduler_.push_and_run(runner);
  }

  job_scheduler scheduler_;
  worker& worker_;
  UserState state_;
  std::string uid_;
  uint16_t room_id_;
};

template <>
void session::on_recv(ReqLogin& msg);

template <>
void session::on_recv(ReqLogout& msg);
}  // namespace session
}  // namespace user
}  // namespace rsp
