/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "rsplib/buffer/shared_const_buffer.hpp"
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/serializer.hpp"
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
using raw_buffer = message::raw_buffer;

class tcp_connection;
using connection_ptr = std::shared_ptr<tcp_connection>;
using dispatcher = message::message_dispatcher_interface;

// https://www.boost.org/doc/libs/1_83_0/doc/html/boost_asio/net_ts.html
// looks like many things changed in modern (including coruitine)
// after development got some where, let's change as modern form
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static constexpr int kBufBytes = 128;
  static connection_ptr create(boost::asio::io_context* io_context,
                               dispatcher* dispatcher) {
    return std::shared_ptr<tcp_connection>(
        new tcp_connection(io_context, dispatcher));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // std::vector<char> bufvec(len);
    // buffer::shared_mutable_buffer buffer{bufvec};
    // std::array<char, LEN_BYTE> bufarr;
    strand_.post(
        std::bind(&tcp_connection::start_impl, shared_from_this(), len));
  }

  void stop() {
    strand_.dispatch(
        boost::bind(&tcp_connection::stop_impl, shared_from_this()));
  }

  void stop(const boost::system::error_code& ec) {
    strand_.dispatch(
        boost::bind(&tcp_connection::stop_impl, shared_from_this(), ec));
  }

  // no handle for message type, just send buffer
  void send(const raw_buffer& msg) {
    buffer::shared_const_buffer buffer{msg};
    strand_.post(
        std::bind(&tcp_connection::send_impl, shared_from_this(), buffer));
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
      : strand_(*io_context), socket_(*io_context), interpreter_(dispatcher) {}

  void start_impl(size_t len) {
    if (!socket_.is_open()) return;
    lg::logger().info() << "start async read, len:" << len << lg::L_endl;
    auto ptr = std::shared_ptr<std::array<char, kBufBytes>>(
        new std::array<char, kBufBytes>);

    namespace asio = boost::asio;
    auto wrap = asio::bind_executor(
        strand_, std::bind(&tcp_connection::handle_read, shared_from_this(),
                           ptr, ph::_1, ph::_2));
    socket_.async_read_some(asio::buffer(*ptr), wrap);
  }

  void stop_impl() {
    // https://stackoverflow.com/questions/12794107/why-do-i-need-strand-per-connection-when-using-boostasio/12801042#12801042
    namespace asio = boost::asio::ip;
    // for now, allow serverside close without restriction.
    if (sent_shutdown_ || !socket_.is_open()) {
      return;
    }

    lg::logger().trace() << "run" << lg::L_endl;
    boost::system::error_code shutdown_ec;
    socket_.shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    // if (shutdown_ec)
    //   lg::logger().debug() << "shutdown error" << shutdown_ec
    //                        << shutdown_ec.message() << lg::L_endl;
    sent_shutdown_ = true;
    lg::logger().debug() << "activate close" << lg::L_endl;
  }

  void stop_impl(const boost::system::error_code& ec) {
    if (!socket_.is_open()) return;

    lg::logger().trace() << "run:" << ec.message() << lg::L_endl;
    auto& logger = lg::logger();
    namespace asio = boost::asio::ip;
    if (sent_shutdown_) {
      logger.debug() << "client also shutdowned" << lg::L_endl;
      socket_.close();
      return;
    }
    logger.debug() << "ec" << ec.message() << ", shutdown" << lg::L_endl;
    boost::system::error_code shutdown_ec;
    socket_.shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    if (shutdown_ec)
      logger.error() << "shutdown error" << shutdown_ec << lg::L_endl;
    sent_shutdown_ = true;
    socket_.close();
  }

  void send_impl(buffer::shared_const_buffer buffer) {
    if (!socket_.is_open()) return;
    namespace asio = boost::asio;
    auto handler = asio::bind_executor(
        strand_, std::bind(&tcp_connection::handle_write, shared_from_this(),
                           buffer, ph::_1, ph::_2));
    asio::async_write(socket_, buffer, handler);
  }

  void handle_write(buffer::shared_const_buffer buffer,
                    const boost::system::error_code& error, size_t bytes) {
    if (sent_shutdown_) {
      return;
    }

    if (boost::asio::error::broken_pipe == error) {
      lg::logger().debug() << "sent or peer recv shutdowned";
      return;
    }

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
      const std::shared_ptr<std::array<char, kBufBytes>>& arr,
      const boost::system::error_code& error, size_t bytes) {
    if (boost::asio::error::eof == error) {
      lg::logger().debug() << "conn: eof" << lg::L_endl;
      stop(error);
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
    start(kBufBytes);
  }

  boost::asio::io_context::strand strand_;
  tcp::socket socket_;
  conn_interpreter interpreter_;
  std::mutex m_;
  link* link_;
  std::atomic<bool> sent_shutdown_;
};

}  // namespace server
}  // namespace libs
}  // namespace rsp
