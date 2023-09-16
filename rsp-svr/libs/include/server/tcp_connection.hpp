#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace server {

using boost::asio::ip::tcp;
namespace ph = std::placeholders;
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static std::shared_ptr<tcp_connection> create(
      boost::asio::io_context* io_context) {
    return std::shared_ptr<tcp_connection>(new tcp_connection(io_context));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // TODO(@nolleh) std::wrap?
    send("welcome, client");
    data_.reset(new std::vector<char>(len));

    utils::logger::instance().debug("start async read" + std::to_string(len));
    socket_.async_read_some(
        boost::asio::buffer(*data_.get()),
        std::bind(&tcp_connection::handle_read, shared_from_this(), data_,
                  ph::_1, ph::_2));
  }

  void send(const std::string& msg) {
    // TODO(@nolleh) warp?
    utils::logger::instance().debug(msg);
    message_ = msg;
    boost::asio::async_write(
        socket_, boost::asio::buffer(message_),
        std::bind(&tcp_connection::handle_write, shared_from_this(), message_,
                  ph::_1, ph::_2));
  }

  void stop() { socket_.close(); }

 private:
  explicit tcp_connection(boost::asio::io_context* io_context)
      : socket_(*io_context) {}
  void handle_write(std::string& s, const boost::system::error_code& error,
                    size_t bytes) {
    utils::logger::instance().debug(
        "write message size(" + std::to_string(bytes) + "), message:" + s);
  }
  void handle_read(std::shared_ptr<std::vector<char>> data,
                   const boost::system::error_code& error, size_t bytes) {
    std::string s(data->begin(), data->end());
    utils::logger::instance().debug("read...message size" +
                                    std::to_string(bytes) + "message:" + s);
    start(std::stoi(s));
  }

  tcp::socket socket_;
  std::string message_;
  // TODO(@nolleh) refact
  std::shared_ptr<std::vector<char>> data_;
};
}  // namespace server
}  // namespace libs
}  // namespace rsp
