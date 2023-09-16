#include <map>
#include <memory>
#include <utility>
#include "session/session.hpp"
#include "server/server_event.hpp"
#include "login.pb.h"

namespace rsp {
namespace user {
namespace session {

class session_manager: public server::server_event {
 public:
  static session_manager& instance() {
    std::call_once(session_manager::s_flag, []() {
      session_manager::s_instance.reset(new session_manager());
    });
    return *session_manager::s_instance;
  }

  void add_session(session s) {
    sessions_[&s].reset(new session(std::move(s)));
  }

  void remove_session(session* s) {
    sessions_.erase(s);
  }

  void on_conn_created(std::shared_ptr<server::tcp_connection>& conn) {
    session s{conn};
    ReqLogin login;
    login.set_uid("nolleh");

    s.send_message(login);
    add_session(s);
  }
  void on_conn_closed(std::shared_ptr<server::tcp_connection>& conn) {
    // remove_session(s);
  }

 private:
  session_manager() {}
  static std::once_flag s_flag;
  static std::unique_ptr<session_manager> s_instance;
  std::map<session*, std::unique_ptr<session>> sessions_;
};

}  // namespace session
}  // namespace user
}  // namespace rsp
