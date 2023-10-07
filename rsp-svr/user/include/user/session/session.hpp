/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
// logged in users
#include <memory>

#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_connection.hpp"

namespace rsp {
namespace user {
namespace session {

namespace server = libs::server;
using link = libs::link::link;
using job_scheduler = libs::job::job_scheduler;

class session;
using session_ptr = std::shared_ptr<session>;

enum class UserState {
  kLogouted,
  kLoggedIn,
  kDetached,
  kInRoom,
};

namespace lg = rsp::libs::logger;
class session : public link, public std::enable_shared_from_this<session> {
 public:
  explicit session(server::connection_ptr conn)
      : link(conn),
        // scheduler_(shared_from_this()),
        // scheduler_(std::dynamic_pointer_cast<link_ptr>(shared_from_this())),
        state_(UserState::kLogouted) {
    session* self = this;
    conn->attach_link(self);
  }

  ~session() {
    // session destroy meaning logout
  }

  void on_connected() override {
    // TODO(@nolleh) notify to other servers that user attached
  }

  // connection was closed
  void on_closed() override {
    // TODO(@nolleh) notify to other servers that user detached
  }

  template <typename Message>
  void on_recv(Message&& msg) {
    lg::logger().error() << "session - on_recv, unknown message" << lg::L_endl;
    // throw std::exception();
  }

  uint16_t room_id() {
    return room_id_;
  }

 private:
  job_scheduler scheduler_;
  UserState state_;
  uint16_t room_id_;
};

template <>
void session::on_recv(ReqLogin& msg);

}  // namespace session
}  // namespace user
}  // namespace rsp
