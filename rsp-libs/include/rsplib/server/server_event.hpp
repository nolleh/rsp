/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <memory>

#include "rsplib/server/tcp_connection.hpp"

namespace rsp {
namespace libs {
namespace server {
class server_event {
 public:
  virtual void on_conn_created(const connection_ptr& conn) = 0;
  virtual void on_conn_closed(const connection_ptr& conn) = 0;
};
}  // namespace server
}  // namespace libs
}  // namespace rsp
