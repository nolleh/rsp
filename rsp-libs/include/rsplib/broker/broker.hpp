
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "rsplib/broker/broker_interface.hpp"
#include "rsplib/broker/cast_type.hpp"

namespace rsp {
namespace libs {
namespace broker {

class broker {
 public:
  static std::shared_ptr<broker_interface> s_create_publisher(
      CastType type, const std::string& service_name, const uint8_t context,
      const std::string& addr);

  static std::shared_ptr<broker_interface> s_create_subscriber(
      CastType type, const std::string& service_name, const uint8_t context,
      const std::string& addr, const std::string& topic = nullptr);
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
