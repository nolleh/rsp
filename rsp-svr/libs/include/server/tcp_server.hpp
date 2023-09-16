#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>

#include "server/tcp_connection.hpp"
#include "thread/thread_pool.hpp"
#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace server {

class tcp_server {
  using tcp = boost::asio::ip::tcp;

 public:
  static const int LEN_BYTE = 1;
  explicit tcp_server()
      : acceptor_threads_(2),
        worker_threads_(10),
        acceptor_(*acceptor_threads_.io_context(),
                  tcp::endpoint(tcp::v4(), 8080)) {}

  ~tcp_server() { stop(); }

  void start() {
    acceptor_threads_.start();
    worker_threads_.start();
    start_accept();
    
    worker_threads_.join();
    acceptor_threads_.join();
  }

  void stop() {
    worker_threads_.stop();
    acceptor_threads_.stop();
  }

  void start_accept() {
    std::shared_ptr<tcp_connection> new_connection =
        tcp_connection::create(worker_threads_.io_context());

    utils::logger::instance().info("start accepting");
    acceptor_.async_accept(new_connection->socket(),
                           std::bind(&tcp_server::handle_accept, this,
                                     new_connection, std::placeholders::_1));
  }

 private:
  void handle_accept(std::shared_ptr<tcp_connection> new_connection,
                     const boost::system::error_code& error) {
    if (!error) {
      new_connection->start(LEN_BYTE);
    }

    // TODO(@nolleh) seperate pool
    start_accept();
  }
  // too make sure live longer than thread_pool
  thread::thread_pool acceptor_threads_;
  thread::thread_pool worker_threads_;
  tcp::acceptor acceptor_;
};
}  // namespace server
}  // namespace libs
}  // namespace rsp
