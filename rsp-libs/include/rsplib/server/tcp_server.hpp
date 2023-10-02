#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>
#include <set>

#include "rsplib/logger/logger.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/server/server_event.hpp"
#include "rsplib/server/tcp_connection.hpp"
#include "rsplib/thread/thread_pool.hpp"

namespace rsp {
namespace libs {
namespace server {

namespace lg = logger;

class tcp_server {
  using tcp = boost::asio::ip::tcp;
  using dispatcher = message::message_dispatcher_interface;
  // using logger = logger::logger;

 public:
  static const int LEN_BYTE = 128;
  explicit tcp_server(dispatcher* dispatcher)
      : acceptor_threads_(2),
        io_threads_(10),
        acceptor_(*acceptor_threads_.io_context(),
                  tcp::endpoint(tcp::v4(), 8080)),
        dispatcher_(dispatcher) {
  }

  ~tcp_server() { stop(); }

  void subscribe(server_event* event) { event_subscribers_.insert(event); }
  void notify_on_created(std::shared_ptr<tcp_connection> conn) {
    for (auto s : event_subscribers_) {
      s->on_conn_created(conn);
    }
  }
  void unsubscribe(server_event* event) { event_subscribers_.erase(event); }

  void start() {
    acceptor_threads_.start();
    io_threads_.start();
    start_accept();

    acceptor_threads_.join();
    io_threads_.join();
  }

  void stop() {
    acceptor_threads_.stop();
    io_threads_.stop();
  }

  void start_accept() {
    std::shared_ptr<tcp_connection> new_connection =
        tcp_connection::create(io_threads_.io_context(), dispatcher_);

    lg::logger().info() << "start accepting" << lg::L_endl;
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

    notify_on_created(new_connection);
    start_accept();
  }

  thread::thread_pool acceptor_threads_;
  thread::thread_pool io_threads_;
  tcp::acceptor acceptor_;
  dispatcher* dispatcher_;
  std::set<server_event*> event_subscribers_;
};
}  // namespace server
}  // namespace libs
}  // namespace rsp
