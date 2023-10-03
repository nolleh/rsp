/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <string>

namespace rsp {
namespace libs {
namespace logger {

enum class log_level { TRACE, DEBUG, INFO, WARN, ERROR };

inline std::string represent_level(const log_level& level) {
  switch (level) {
    case log_level::TRACE:
      return "TRACE";
    case log_level::DEBUG:
      return "DEBUG";
    case log_level::INFO:
      return "INFO";
    case log_level::WARN:
      return "WARN";
    case log_level::ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
