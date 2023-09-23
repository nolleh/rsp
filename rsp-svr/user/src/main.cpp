#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
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

  try {
    std::cout << func(1, 2) << std::endl;

    server::tcp_server server;
    user_server::acceptor acceptor;
    server.subscribe(&acceptor);
    server.start();
    // TODO(@nolleh) more elegant way
    server.unsubscribe(&acceptor);
  } catch (std::exception &e) {
    logger::instance().error("exception ocurred", e.what());
  }
}
