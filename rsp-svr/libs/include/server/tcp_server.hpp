#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>

#include "server/tcp_connection.hpp"
namespace rsp {
namespace libs {
namespace server {

class tcp_server {
  using tcp = boost::asio::ip::tcp;

 public:
  static const int LEN_BYTE = 1;
  explicit tcp_server(boost::asio::io_context* io_context)
      : io_context_(io_context),
        acceptor_(*io_context, tcp::endpoint(tcp::v4(), 8080)) {
    start_accept();
  }

  void start_accept() {
    std::shared_ptr<tcp_connection> new_connection =
        tcp_connection::create(io_context_);

    acceptor_.async_accept(
        new_connection->socket(),
        std::bind(&tcp_server::handle_accept, this, new_connection,
                  std::placeholders::_1));
  }

 private:
  void handle_accept(std::shared_ptr<tcp_connection> new_connection,
                     const boost::system::error_code& error) {
    if (!error) {
      new_connection->start(LEN_BYTE);
    }

    // TODO (@nolleh) seperate pool
    start_accept();
  }
  boost::asio::io_context* io_context_;
  tcp::acceptor acceptor_;
};
}  // namespace server
}  // namespace libs
}  // namespace rsp
