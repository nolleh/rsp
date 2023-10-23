/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "rsplib/logger/logger.hpp"

int main() {
  std::cout << "hello, world" << std::endl;
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  logger.info() << "started room server" << lg::L_endl;
  return 0;
}
