/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <bitset>
#include <map>
#include <memory>
#include <mutex>
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
    std::lock_guard<std::mutex> lock(m_);
    sessions_[s_ptr->conn_ptr()] = s_ptr;
    const auto uid = s_ptr->uid();
    if (!uid.empty()) uid_sessions_[uid] = s_ptr;
  }

  void add_session(const server::connection_ptr& conn) {
    std::lock_guard<std::mutex> lock(m_);
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
    std::lock_guard<std::mutex> lock(m_);
    auto iter = sessions_.find(conn);
    if (iter == sessions_.end()) return;
    const auto uid = iter->second->uid();
    if (!uid.empty()) {
      auto uid_iter = uid_sessions_.find(uid);
      if (uid_iter != uid_sessions_.end() && uid_iter->second == iter->second)
        uid_sessions_.erase(uid_iter);
    }
    sessions_.erase(iter);
  }

  std::shared_ptr<session> find_session(const std::string& uid) {
    std::lock_guard<std::mutex> lock(m_);
    auto iter = uid_sessions_.find(uid);
    if (uid_sessions_.end() == iter) {
      return nullptr;
    }

    return iter->second;
  }

 private:
  session_manager() {}
  static std::once_flag s_flag;
  static std::unique_ptr<session_manager> s_instance;

  std::mutex m_;
  std::map<server::connection_ptr, session_ptr> sessions_;
  std::map<std::string, session_ptr> uid_sessions_;
};

}  // namespace session
}  // namespace user
}  // namespace rsp
