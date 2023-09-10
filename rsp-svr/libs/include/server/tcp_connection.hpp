#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace server {

using boost::asio::ip::tcp;
class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
 public:
  static std::shared_ptr<tcp_connection> create(
      boost::asio::io_context* io_context) {
    return std::shared_ptr<tcp_connection>(new tcp_connection(io_context));
  }

  tcp::socket& socket() { return socket_; }

  void start(size_t len) {
    // TODO(@nolleh) std::wrap?
    data_.reset(new std::vector<char>(len));

    socket_.async_read_some(
        boost::asio::buffer(*data_.get()),
        std::bind(&tcp_connection::handle_read, shared_from_this(), data_,
                  std::placeholders::_1, std::placeholders::_2));
  }

  void send(const std::string& msg) {
    // TODO(@nolleh) warp?
    message_ = msg;
    boost::asio::async_write(
        socket_, boost::asio::buffer(message_),
        std::bind(&tcp_connection::handle_write, shared_from_this(),
                  std::placeholders::_1, std::placeholders::_2));
  }

  void stop() {
    socket_.close();
  }

 private:
  explicit tcp_connection(boost::asio::io_context* io_context)
      : socket_(*io_context) {}
  void handle_write(const boost::system::error_code& error, size_t bytes) {}
  void handle_read(std::shared_ptr<std::vector<char>> data,
                   const boost::system::error_code& error, size_t bytes) {
    std::string s(data->begin(), data->end());
    utils::logger::get_instance().debug("read...message size" +
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
