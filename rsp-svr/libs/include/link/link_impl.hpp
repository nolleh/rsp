#include <string>

#include "server/tcp_connection.hpp"

namespace rsp {
namespace libs {
namespace link {

class link_impl {
 public:
  static const int LEN_BYTE = 1;
  void start() { conn_.start(LEN_BYTE); }

  void send(std::string msg) { conn_.send(msg); }

  void stop() { conn_.stop(); }

 private:
  server::tcp_connection conn_;
};
}  // namespace link
}  // namespace libs
}  // namespace rsp
