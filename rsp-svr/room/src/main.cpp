/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "proto/common/ping.pb.h"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"

int main() {
  std::cout << "hello, world" << std::endl;
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  namespace br = rsp::libs::broker;
  auto sub =
      br::broker::s_create_subscriber(CastType::kAnyCast, "user", 1, "topic");
  sub->start();

  logger.info() << "waiting message is ready" << lg::L_endl;
  sleep(3);
  auto buffer = sub->recv("topic").get();
  namespace msg = rsp::libs::message;

  auto ping = Ping();
  auto success = msg::serializer::deserialize(buffer, &ping);
  if (!success) {
    logger.error() << "failed to deserialize ping" << lg::L_endl;
  } else {
    logger.debug() << "success to deserialize ping" << lg::L_endl;
  }

  logger.info() << "started room server" << lg::L_endl;
  return 0;
}
