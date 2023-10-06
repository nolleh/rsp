/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include <boost/asio.hpp>

#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_server.hpp"
#include "user/config.h"
#include "user/message/message_dispatcher.hpp"
#include "user/server/connection.hpp"

int main() {
  namespace server = rsp::libs::server;
  namespace user_server = rsp::user::server;
  namespace lg = rsp::libs::logger;
  using dispatcher = rsp::user::message::message_dispatcher;

  auto& logger = lg::logger(lg::log_level::kTrace);
  logger.info() << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
                << lg::L_endl;

  try {
    dispatcher dispatcher;
    server::tcp_server server{&dispatcher};
    user_server::acceptor acceptor;
    server.subscribe(&acceptor);
    server.start();
    // TODO(@nolleh) more elegant wayk
    server.unsubscribe(&acceptor);
  } catch (std::exception& e) {
    logger.error() << "exception ocurred" << e.what() << lg::L_endl;
  }
}
