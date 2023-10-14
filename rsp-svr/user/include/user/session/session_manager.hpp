/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/types.hpp"
#include "rsplib/server/server_event.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace session {

namespace message = rsp::libs::message;

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
    const auto s = std::make_shared<session>(conn);
    s->start();
    sessions_[conn] = s;
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

  std::map<server::connection_ptr, std::shared_ptr<session>> sessions_;
};

}  // namespace session
}  // namespace user
}  // namespace rsp
