/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "rsplib/logger/logger.hpp"
#include "rsplib/broker/broker.hpp"

int main() {
  std::cout << "hello, world" << std::endl;
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  namespace br = rsp::libs::broker;
  auto pub = br::broker::s_create_publisher(CastType::kAnyCast, "room", 1);
  logger.info() << "started room server" << lg::L_endl;
  return 0;
}
