/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include <boost/asio.hpp>

#include "proto/common/ping.pb.h"
#include "rsplib/broker/broker.hpp"
#include "rsplib/debug/tracer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/server/tcp_server.hpp"
#include "user/config.h"
#include "user/message/message_dispatcher.hpp"
#include "user/server/acceptor.hpp"
#include "user/server/worker.hpp"
#include "user/intranet/intranet.hpp"

int main() {
  namespace server = rsp::libs::server;
  namespace user_server = rsp::user::server;
  namespace lg = rsp::libs::logger;
  namespace br = rsp::libs::broker;
  using dispatcher = rsp::user::message::message_dispatcher;

  rsp::libs::tracer::install();

  auto& logger = lg::logger(lg::log_level::kTrace);
  logger.info() << "User Version: " << User_VERSION_MAJOR << User_VERSION_MINOR
                << lg::L_endl;

  try {
    auto& intranet = rsp::user::intranet::instance();
    intranet.start();

    dispatcher dispatcher;
    server::tcp_server server{&dispatcher};
    user_server::acceptor acceptor;
    server.subscribe(&acceptor);
    user_server::worker::instance();  // initialize when start server, worker;
    server.start();
    // TODO(@nolleh) more elegant way
    server.unsubscribe(&acceptor);
  } catch (std::exception& e) {
    logger.error() << "exception occurred" << e.what() << lg::L_endl;
  }
}
