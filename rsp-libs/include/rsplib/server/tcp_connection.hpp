#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "rsplib/buffer/shared_const_buffer.hpp"
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {

namespace link {
class link;
}
namespace server {

using boost::asio::ip::tcp;

namespace ph = std::placeholders;
using conn_interpreter = message::conn_interpreter;
namespace lg = logger;
using link = link::link;

class tcp_connection;
using connection_ptr = std::shared_ptr<tcp_connection>;
using dispatcher = message::message_dispatcher_interface;

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static constexpr int LEN_BYTE = 128;
  static connection_ptr create(boost::asio::io_context* io_context,
                               dispatcher* dispatcher) {
    return std::shared_ptr<tcp_connection>(
        new tcp_connection(io_context, dispatcher));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // TODO(@nolleh) std::wrap?
    // std::vector<char> bufvec(len);
    // buffer::shared_mutable_buffer buffer{bufvec};
    // std::array<char, LEN_BYTE> bufarr;
    lg::logger().info() << "start async read" << len << lg::L_endl;
    auto ptr = std::shared_ptr<std::array<char, LEN_BYTE>>(
        new std::array<char, LEN_BYTE>);
    socket_.async_read_some(boost::asio::buffer(*ptr),
                            std::bind(&tcp_connection::handle_read,
                                      shared_from_this(), ptr, ph::_1, ph::_2));
  }

  void stop() { socket_.close(); }

  template <typename Message>
  void send(Message msg) {
    std::string bytes;
    auto success = msg.SerializeToString(&bytes);
    if (!success) {
      lg::logger().error() << "failed to serialize message" << lg::L_endl;
    }
    // REMARK(@nolleh) looks like template speiclaization is hard to deduce &
    // type. to avoid unneccessary copy, invoke impl function
    send_impl(bytes);
  }

  void attach_link(link* link) {
    std::lock_guard<std::mutex> lock(m_);
    link_ = link;
    interpreter_.attach_link(link);
  }

  void detach_link() {
    std::lock_guard<std::mutex> lock(m_);
    link_ = nullptr;
  }

 private:
  // TODO(@nolleh) consider options for linger / nagle
  explicit tcp_connection(boost::asio::io_context* io_context,
                          dispatcher* dispatcher)
      : socket_(*io_context), interpreter_(dispatcher) {}

  void send_impl(const std::string& msg) {
    // TODO(@nolleh) warp?
    lg::logger().debug() << msg << lg::L_endl;
    buffer::shared_const_buffer buffer{msg};
    boost::asio::async_write(
        socket_, buffer,
        std::bind(&tcp_connection::handle_write, shared_from_this(), buffer,
                  ph::_1, ph::_2));
  }

  void handle_write(buffer::shared_const_buffer buffer,
                    const boost::system::error_code& error, size_t bytes) {
    if (error) {
      lg::logger().error() << "failed to async_write: " + error.message()
                           << lg::L_endl;
      return;
    }
    lg::logger().trace() << "conn: write message size(" +
                                std::to_string(bytes) + ")"
                         << lg::L_endl;
  }

  void handle_read(
      // buffer::shared_mutable_buffer buffer,
      const std::shared_ptr<std::array<char, LEN_BYTE>>& arr,
      const boost::system::error_code& error, size_t bytes) {
    if (boost::asio::error::eof == error) {
      lg::logger().debug() << "conn: closed" << lg::L_endl;
      stop();
      return;
    }

    if (boost::asio::error::operation_aborted == error) {
      lg::logger().debug()
          << "conn: canceld operation (aborted conn) socket is opend?:" +
                 std::to_string(socket_.is_open())
          << lg::L_endl;
      return;
    }

    if (error) {
      lg::logger().error() << "failed to async_read: " + error.message()
                           << lg::L_endl;
      // start(1);
      return;
    }

    lg::logger().trace() << "conn: read...message size(" << bytes << ")"
                         << lg::L_endl;
    interpreter_.handle_buffer(*arr, bytes);
    start(LEN_BYTE);
  }

  tcp::socket socket_;
  conn_interpreter interpreter_;
  std::mutex m_;
  link* link_;
};

// TODO(@nolleh) refactor
template <>
void tcp_connection::send(const char* msg);
template <>
void tcp_connection::send(std::basic_string<char> msg);
template <>
void tcp_connection::send(std::vector<char> msg);

}  // namespace server
}  // namespace libs
}  // namespace rsp
