/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

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

using conn_interpreter = message::conn_interpreter;
namespace ph = std::placeholders;
namespace lg = logger;
using link = link::link;
using raw_buffer = message::raw_buffer;

class tcp_connection;
using connection_ptr = std::shared_ptr<tcp_connection>;
using close_handler = std::function<void(const connection_ptr&)>;
using dispatcher = message::message_dispatcher_interface;

// https://www.boost.org/doc/libs/1_83_0/doc/html/boost_asio/net_ts.html
// looks like many things changed in modern (including coruitine)
// after development got some where, let's change as modern form
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static constexpr int kBufBytes = 128;
  static connection_ptr create(boost::asio::io_context* io_context,
                               dispatcher* dispatcher,
                               close_handler on_closed = {}) {
    return std::shared_ptr<tcp_connection>(
        new tcp_connection(io_context, dispatcher, std::move(on_closed)));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // std::vector<char> bufvec(len);
    // buffer::shared_mutable_buffer buffer{bufvec};
    // std::array<char, LEN_BYTE> bufarr;
    lg::logger().debug() << "post impl" << lg::L_endl;
    strand_.post(
        std::bind(&tcp_connection::start_impl, shared_from_this(), len));
  }

  void stop(bool force_close = false) {
    lg::logger().debug() << "post stop impl, force:" << force_close
                         << lg::L_endl;
    auto handler = std::bind(&tcp_connection::active_stop_impl,
                             shared_from_this(), force_close);
    if (force_close) {
      strand_.dispatch(std::move(handler));
    } else {
      // Preserve the order of sends already posted, even from this strand.
      strand_.post(std::move(handler));
    }
  }

  void stop(const boost::system::error_code& ec) {
    lg::logger().debug() << "post impl" << lg::L_endl;
    strand_.dispatch(
        std::bind(&tcp_connection::stop_impl, shared_from_this(), ec));
  }

  // no handle for message type, just send buffer
  void send(const raw_buffer& msg) {
    lg::logger().debug() << "send post impl" << lg::L_endl;
    shared_const_buffer buffer{msg};
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
                          dispatcher* dispatcher, close_handler on_closed)
      : strand_(*io_context),
        socket_(*io_context),
        interpreter_(dispatcher),
        on_closed_(std::move(on_closed)) {}

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

  void active_stop_impl(const bool force) {
    // https://stackoverflow.com/questions/12794107/why-do-i-need-strand-per-connection-when-using-boostasio/12801042#12801042
    // for now, allow serverside close without restriction.
    if (closed_ || (!force && graceful_close_requested_)) {
      return;
    }

    if (!socket_.is_open() || force) {
      finish_close();
      return;
    }

    graceful_close_requested_ = true;
    if (!write_queue_.empty()) return;
    shutdown_send_impl();
  }

  void shutdown_send_impl() {
    namespace asio = boost::asio::ip;
    lg::logger().trace() << "run" << lg::L_endl;
    boost::system::error_code shutdown_ec;
    socket_.shutdown(asio::tcp::socket::shutdown_send, shutdown_ec);
    // if (shutdown_ec)
    //   lg::logger().debug() << "shutdown error" << shutdown_ec
    //                        << shutdown_ec.message() << lg::L_endl;
    lg::logger().debug() << "activate close, sent(" << sent_shutdown_
                         << "), shutdown_ec:" << shutdown_ec.message()
                         << ", open:" << socket_.is_open() << lg::L_endl;
    sent_shutdown_ = true;
    if (shutdown_ec) finish_close();
  }

  void stop_impl(const boost::system::error_code& ec) {
    lg::logger().trace() << "run:" << ec.message() << lg::L_endl;
    finish_close();
  }

  void finish_close() {
    if (closed_) return;
    closed_ = true;

    boost::system::error_code ignored;
    if (socket_.is_open()) {
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
      socket_.close(ignored);
    }
    write_queue_.clear();

    auto on_closed = std::move(on_closed_);
    if (on_closed) on_closed(shared_from_this());
  }

  void send_impl(shared_const_buffer buffer) {
    if (closed_ || graceful_close_requested_ || !socket_.is_open()) return;
    const bool write_in_progress = !write_queue_.empty();
    write_queue_.push_back(std::move(buffer));
    if (write_in_progress) return;

    write_next();
  }

  void write_next() {
    namespace asio = boost::asio;
    auto handler = asio::bind_executor(
        strand_, std::bind(&tcp_connection::handle_write, shared_from_this(),
                           ph::_1, ph::_2));
    asio::async_write(socket_, write_queue_.front(), handler);
  }

  void handle_write(const boost::system::error_code& error, size_t bytes) {
    if (closed_) return;

    if (boost::asio::error::broken_pipe == error) {
      lg::logger().debug() << "sent or peer recv shutdowned";
      finish_close();
      return;
    }

    if (error) {
      lg::logger().error() << "failed to async_write: " + error.message()
                           << lg::L_endl;
      finish_close();
      return;
    }
    lg::logger().trace() << "conn: write message size(" +
                                std::to_string(bytes) + ")"
                         << lg::L_endl;
    write_queue_.pop_front();
    if (!write_queue_.empty()) {
      write_next();
    } else if (graceful_close_requested_) {
      shutdown_send_impl();
    }
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
      if (!closed_) finish_close();
      return;
    }

    if (error) {
      lg::logger().error() << "failed to async_read: " + error.message()
                           << lg::L_endl;
      stop(error);
      return;
    }

    lg::logger().trace() << "conn: read...message size(" << bytes << ")"
                         << lg::L_endl;
    if (!interpreter_.handle_buffer(*arr, bytes)) {
      lg::logger().warn() << "invalid message frame" << lg::L_endl;
      stop(true);
      return;
    }
    start(kBufBytes);
  }

  boost::asio::io_context::strand strand_;
  tcp::socket socket_;
  conn_interpreter interpreter_;
  std::mutex m_;
  link* link_;
  std::deque<shared_const_buffer> write_queue_;
  close_handler on_closed_;
  bool graceful_close_requested_{false};
  bool sent_shutdown_{false};
  bool closed_{false};
};

}  // namespace server
}  // namespace libs
}  // namespace rsp
