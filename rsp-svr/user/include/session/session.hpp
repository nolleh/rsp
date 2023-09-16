#pragma once
// logged in users
#include <memory>

#include "server/tcp_connection.hpp"

namespace rsp {
namespace server = libs::server;
namespace user {
namespace session {

class session {
 public:
  session(std::shared_ptr<server::tcp_connection> conn) : connection_(conn) {}
  ~session() {
    // logout meaning disconnect
    connection_->stop();
  }

  // session(session&& r) : connection_(std::move(r.connection_)) {}

  template <typename Message>
  void send_message(Message&& msg) {
    connection_->send(msg);
  }

  template <typename Message>
  void on_recv(Message&& msg) {}

  // connection was closed
  void on_closed() {}

 private:
  std::shared_ptr<server::tcp_connection> connection_;
  // std::unique_ptr<server::tcp_connection> connection_;
};
}  // namespace session
}  // namespace user
}  // namespace rsp
