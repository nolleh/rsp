#pragma once
#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/util.hpp"
#include "rsplib/server/server_event.hpp"
#include "session/session.hpp"

namespace rsp {
namespace user {
namespace session {

namespace message = rsp::libs::message;

class session_manager {
  using logger = rsp::libs::logger::logger;

 public:
  static session_manager& instance() {
    std::call_once(session_manager::s_flag, []() {
      session_manager::s_instance.reset(new session_manager());
    });
    return *session_manager::s_instance;
  }

  void add_session(const session& session) { add_session(session.conn_ptr()); }

  void add_session(const server::connection_ptr& conn) {
    sessions_[conn].reset(new session(conn));
    ResLogin login;
    login.set_uid("nolleh");
    login.set_success(true);

    auto content_len = login.ByteSizeLong();
    std::cout << content_len << "," << sizeof(content_len) << std::endl;
    std::vector<char> message;
    message::mset(&message, content_len);
    message::mset(&message, static_cast<int>(MessageType::RES_LOGIN));
    const auto str = login.SerializeAsString();
    message.insert(message.end(), str.begin(), str.end());
    sessions_[conn]->send_message(message);
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
