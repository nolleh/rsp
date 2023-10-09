/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>

#include "rsplib/server/tcp_connection.hpp"

namespace rsp {
namespace libs {
namespace link {

using connection_ptr = rsp::libs::server::connection_ptr;

// eventhandler
class link {
 public:
  explicit link(connection_ptr conn) : connection_(conn) {}
  ~link() {
    // stop();
  }

  virtual void on_connected() = 0;
  virtual void on_disconnected() = 0;
  virtual void on_closed() { connection_->detach_link(); }

  const server::connection_ptr& conn_ptr() const { return connection_; }

  template <typename Message>
  void on_recv(Message&& msg) {}

  template <typename Message>
  void send(const Message& msg) {
    connection_->send(msg);
  }

  void stop() { connection_->stop(); }

 private:
  // TODO(@nolleh) change to conn
  const connection_ptr connection_;
};
}  // namespace link
}  // namespace libs
}  // namespace rsp
