/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <string>

#include "room/intranet/message_dispatcher.hpp"
#include "room/intranet/message_trait.hpp"
#include "room/intranet/user_sender.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;

using address = std::string;
class user_topology {
 public:
  explicit user_topology(intranet* intranet)
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(room_message_handler(intranet)) {}

 private:
  lg::s_logger& logger_;
  message_dispatcher<user_topology> dispatcher_;
  std::map<address, std::shared_ptr<br::broker_interface>> user_servers_;
  room_message_handler message_handler_;
};
}  // namespace room
}  // namespace rsp
