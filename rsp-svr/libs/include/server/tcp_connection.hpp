#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "buffer/shared_const_buffer.hpp"
#include "buffer/shared_mutable_buffer.hpp"
#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace server {

using boost::asio::ip::tcp;
namespace ph = std::placeholders;
class tcp_connection;

using connection_ptr = std::shared_ptr<tcp_connection>;
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static connection_ptr create(boost::asio::io_context* io_context) {
    return std::shared_ptr<tcp_connection>(new tcp_connection(io_context));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // TODO(@nolleh) std::wrap?
    // send("welcome, client");

    shared_mutable_buffer buffer{std::vector<char>(len)};
    utils::logger::instance().debug("start async read" + std::to_string(len));
    socket_.async_read_some(
        buffer, std::bind(&tcp_connection::handle_read, shared_from_this(),
                          buffer, ph::_1, ph::_2));
  }

  void stop() { socket_.close(); }

  template <typename Message>
  void send(Message msg) {
    std::string bytes;
    auto success = msg.SerializeToString(&bytes);
    if (!success) {
      utils::logger::instance().error("failed to serialize message");
    }
    // send(bytes);
    // REMARK(@nolleh) looks like template speiclaization is hard to deduce &
    // type. to avoid unneccessary copy, invoke impl function
    send_impl(bytes);
  }

 private:
  explicit tcp_connection(boost::asio::io_context* io_context)
      : socket_(*io_context) {}

  void send_impl(const std::string& msg) {
    // TODO(@nolleh) warp?
    utils::logger::instance().debug(msg);
    shared_const_buffer buffer{msg};
    boost::asio::async_write(
        socket_, buffer,
        std::bind(&tcp_connection::handle_write, shared_from_this(), buffer,
                  ph::_1, ph::_2));
  }
  void handle_write(shared_const_buffer buffer,
                    const boost::system::error_code& error, size_t bytes) {
    // const std::string s{buffer.begin(), buffer.end()};
    utils::logger::instance().trace("conn: write message size(" +
                                    std::to_string(bytes) + "), message:");
  }

  void handle_read(shared_mutable_buffer buffer,
                   const boost::system::error_code& error, size_t bytes) {
    if (boost::asio::error::eof) {
      utils::logger::instance().debug("conn: closed");
      stop();
      return;
    }

    // std::string s(buffer.begin(), buffer.end());
    utils::logger::instance().trace("conn: read...message size" +
                                    std::to_string(bytes) + "message:");
    // start(std::stoi(s));
    start(1);
  }

  tcp::socket socket_;
};

template <>
void tcp_connection::send(const char* msg);
template <>
void tcp_connection::send(std::basic_string<char> msg);

}  // namespace server
}  // namespace libs
}  // namespace rsp
