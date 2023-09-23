#include "rsplib/server/server_event.hpp"
#include "session/session.hpp"
#include "session/session_manager.hpp"

namespace rsp {
namespace user {
namespace server {
namespace event = libs::server;

class acceptor : public event::server_event {
 public:
  acceptor() : manager_(session::session_manager::instance()) {}
  void on_conn_created(const event::connection_ptr& conn) {
    manager_.add_session(conn);
  }
  void on_conn_closed(const event::connection_ptr& conn) {
    manager_.remove_session(conn);
  }

 private:
  session::session_manager& manager_;
};
}  // namespace server
}  // namespace user
}  // namespace rsp
