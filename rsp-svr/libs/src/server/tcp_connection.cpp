#include "server/tcp_connection.hpp"

namespace rsp {
namespace libs {
namespace server {

template <>
void tcp_connection::send(const char* msg) {
  std::string str(msg);
  // send(str);
  logger::instance().debug(msg);
  shared_const_buffer buffer{msg};
  boost::asio::async_write(
      socket_, buffer,
      std::bind(&tcp_connection::handle_write, shared_from_this(), buffer,
                ph::_1, ph::_2));
}

template <>
void tcp_connection::send(std::basic_string<char> msg) {
  // TODO(@nolleh) warp?
  logger::instance().debug(msg);
  shared_const_buffer buffer{msg};
  boost::asio::async_write(
      socket_, buffer,
      std::bind(&tcp_connection::handle_write, shared_from_this(), buffer,
                ph::_1, ph::_2));
}

}  // namespace server
}  // namespace libs
}  // namespace rsp
