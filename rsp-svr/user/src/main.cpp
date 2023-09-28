#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
#include "message/message_dispatcher.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_server.hpp"
#include "server/connection.hpp"
#include "test.hpp"

int main() {
  std::cout << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
            << std::endl;

  namespace server = rsp::libs::server;
  namespace user_server = rsp::user::server;
  using logger = rsp::libs::logger::logger;
  using dispatcher = rsp::user::message::message_dispatcher;
  try {
    std::cout << func(1, 2) << std::endl;

    // server::tcp_server server{dispatcher::instance()};
    dispatcher dispatcher;
    server::tcp_server server{&dispatcher};
    user_server::acceptor acceptor;
    server.subscribe(&acceptor);
    server.start();
    // TODO(@nolleh) more elegant way
    server.unsubscribe(&acceptor);
  } catch (std::exception &e) {
    logger::instance().error("exception ocurred", e.what());
  }
}
