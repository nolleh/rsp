/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "user/intranet/types.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;

class user_receiver {
 public:
  user_receiver() : logger_(lg::logger()) {
    subscriber_ = br::broker::s_create_subscriber(CastType::kAnyCast, "room", 1,
                                                  "tcp://*:5558", "topic");
  }

  Address addr() {
    // TODO(@nolleh) how to get my ip address?
    return "tcp://127.0.0.1:5558";
  }

 private:
  lg::s_logger& logger_;
  std::shared_ptr<br::broker_interface> subscriber_;
};

}  // namespace user
}  // namespace rsp
