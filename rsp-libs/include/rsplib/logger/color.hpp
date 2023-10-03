#pragma once

#include "rsplib/logger/level.hpp"

namespace rsp {
namespace libs {
namespace logger {

enum color {
  C_red = 31,
  C_green = 32,
  C_yellow = 33,
  C_darkBlue = 34,
  C_blue = 36,
  C_gray = 37
};

inline int color_code(const log_level& level) {
  switch (level) {
    case log_level::TRACE:
      return color::C_gray;
    case log_level::DEBUG:
      return color::C_darkBlue;
    case log_level::INFO:
      return color::C_green;
    case log_level::WARN:
      return color::C_yellow;
    case log_level::ERROR:
      return color::C_red;
  }
  return 0;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
