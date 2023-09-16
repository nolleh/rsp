#pragma once
#include <map>
#include <memory>
#include <utility>

#include "login.pb.h"
#include "server/server_event.hpp"
#include "session/session.hpp"

namespace rsp {
namespace user {
namespace session {

class session_manager {
 public:
  static session_manager& instance() {
    std::call_once(session_manager::s_flag, []() {
      session_manager::s_instance.reset(new session_manager());
    });
    return *session_manager::s_instance;
  }

  void add_session(const session& session) { add_session(session.conn_ptr()); }

  void add_session(const server::connection_ptr& conn) {
    session s{conn};
    ReqLogin login;
    login.set_uid("nolleh");
    s.send_message(login);
    s.send_message("hello");
    sessions_[conn].reset(new session(std::move(s)));
  }

  void remove_session(const session& session) {
    remove_session(session.conn_ptr());
  }

  void remove_session(const server::connection_ptr& conn) {
    sessions_.erase(conn);
  }

 private:
  session_manager() {}
  static std::once_flag s_flag;
  static std::unique_ptr<session_manager> s_instance;
  std::map<server::connection_ptr, std::unique_ptr<session>> sessions_;
};

}  // namespace session
}  // namespace user
}  // namespace rsp
