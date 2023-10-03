/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "rsplib/logger/console_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

inline console_logger& logger(log_level level = log_level::TRACE,
                              flags options = L_color) {
  return console_logger::instance(level, options);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
