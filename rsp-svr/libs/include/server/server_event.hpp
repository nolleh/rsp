#pragma once
#include "server/tcp_connection.hpp"

namespace rsp {
namespace libs {
namespace server {
class server_event {
public:
  virtual void on_conn_created(std::shared_ptr<tcp_connection>& conn) = 0;
  virtual void on_conn_closed(std::shared_ptr<tcp_connection>& conn) = 0;
};
}  // namespace server
}  // namespace utils
}  // namespace rsp
