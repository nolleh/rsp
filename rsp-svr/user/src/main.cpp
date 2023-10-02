#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
#include "message/message_dispatcher.hpp"
#include "rsplib/logger/console_logger.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_server.hpp"
#include "server/connection.hpp"
#include "test.hpp"

int main() {
  namespace server = rsp::libs::server;
  namespace user_server = rsp::user::server;
  namespace lg = rsp::libs::logger;
  using dispatcher = rsp::user::message::message_dispatcher;
  auto& logger = lg::console_logger::instance(lg::log_level::INFO);
  logger << lg::log_level::DEBUG << lg::L_time
         << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
         << lg::L_endl;

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
  } catch (std::exception& e) {
    logger << lg::log_level::ERROR << "exception ocurred" << e.what()
           << lg::L_endl;
  }
}
