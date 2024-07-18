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

  void add_session(const session_ptr& s_ptr) {
    sessions_[s_ptr->conn_ptr()] = s_ptr;
    uid_sessions_[s_ptr->uid()] = s_ptr;
  }

  void add_session(const server::connection_ptr& conn) {
    auto iter = sessions_.find(conn);
    if (sessions_.end() != iter) {
      return;
    }

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

  std::shared_ptr<session> find_session(const std::string& uid) {
    auto iter = uid_sessions_.find(uid);
    if (uid_sessions_.end() != iter) {
      return nullptr;
    }

    return iter->second;
  }

 private:
  session_manager() {}
  static std::once_flag s_flag;
  static std::unique_ptr<session_manager> s_instance;

  std::map<server::connection_ptr, session_ptr> sessions_;
  std::map<std::string, session_ptr> uid_sessions_;
};

}  // namespace session
}  // namespace user
}  // namespace rsp
