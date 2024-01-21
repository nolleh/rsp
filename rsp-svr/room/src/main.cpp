/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <iostream>

#include "proto/common/ping.pb.h"
#include "room/intranet/intranet.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"

int main() {
  namespace lg = rsp::libs::logger;
  auto& logger = lg::logger(lg::log_level::kTrace);

  logger.info() << "started room server" << lg::L_endl;
  rsp::room::intranet intranet;
  intranet.start();

  return 0;
}
