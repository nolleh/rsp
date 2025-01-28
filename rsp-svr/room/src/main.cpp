/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "proto/common/ping.pb.h"
#include "room/intranet/intranet.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"
#include "room/so/so_manager.hpp"

int main() {
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  logger.info() << "started room server" << lg::L_endl;
  rsp::room::so_manager::instance().load();

  auto& intranet = rsp::room::intranet::instance();
  intranet.start();

  logger.info() << "room server stopped.." << lg::L_endl;
  return 0;
}
