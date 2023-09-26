#pragma once
// logged in users
#include <memory>

#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/server/tcp_connection.hpp"

namespace rsp {
namespace user {
namespace session {

namespace server = libs::server;
using link = libs::link::link;
using job_scheduler = libs::job::job_scheduler;

class session;
using session_ptr = std::shared_ptr<session>;

using link_ptr = std::shared_ptr<link>;

enum class UserState {
  LOGOUTED,
  LOGGED_IN,
  DETACHED,
  IN_ROOM,
};

class session : public link {
 public:
  explicit session(server::connection_ptr conn)
      : link(conn),
        scheduler_(shared_from_this()),
        // job_scheduler_(std::dynamic_pointer_cast<link_ptr>(shared_from_this()),
        state_(UserState::LOGOUTED) {
    session* me = this;
  }

  ~session() {
    // session destroy meaning logout
  }

  virtual void on_connected() {
    // TODO(@nolleh) notify to other servers that user attached
  }

  // connection was closed
  virtual void on_closed() {
    // TODO(@nolleh) notify to other servers that user detached
  }

  template <typename Message>
  void on_recv(Message&& msg) {
    std::cerr << "session - on_recv, unkown message" << std::endl;
    // throw std::exception();
  }

 private:
  job_scheduler scheduler_;
  UserState state_;
  uint16_t room_id_;
};

template <>
void session::on_recv(ReqLogin&& msg);

}  // namespace session
}  // namespace user
}  // namespace rsp
