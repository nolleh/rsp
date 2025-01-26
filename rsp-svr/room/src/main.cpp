/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "proto/common/ping.pb.h"
#include "room/intranet/intranet.hpp"
#include "room/so/so_factory.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"

int main() {
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  logger.info() << "started room server" << lg::L_endl;
  // TODO(@nolleh) separate manager
  rsp::room::so_factory factory;
  auto contents_so = factory.create();
  contents_so->on_load();

  auto& intranet = rsp::room::intranet::instance();
  intranet.start();

  contents_so->on_unload();
  logger.info() << "room server stopped.." << lg::L_endl;
  return 0;
}
