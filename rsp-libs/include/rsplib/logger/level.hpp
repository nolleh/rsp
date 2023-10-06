/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <string>

namespace rsp {
namespace libs {
namespace logger {

enum class log_level { kTrace, kDebug, kInfo, kWarn, kError };

inline std::string represent_level(const log_level& level) {
  switch (level) {
    case log_level::kTrace:
      return "TRACE";
    case log_level::kDebug:
      return "DEBUG";
    case log_level::kInfo:
      return "INFO";
    case log_level::kWarn:
      return "WARN";
    case log_level::kError:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
