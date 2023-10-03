#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "rsplib/logger/console_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

inline console_logger& logger(log_level level = log_level::TRACE) {
  return console_logger::instance(level);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
