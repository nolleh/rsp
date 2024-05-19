/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

namespace rsp {
namespace libs {
namespace logger {

enum flags {
  L_clearflags,
  L_concat,
  L_time,
  L_flush,
  L_endl,
  L_startWithFlushing,
  L_null,
  L_cout,
  L_space,
  L_color = 16,
  L_tabs = 32,
  L_allwaysFlush = 64,
  L_level = 128
};

// semantic addition and subtraction for flag enum
inline flags operator+=(flags& l_flag, flags r_flag) {
  return l_flag = static_cast<flags>(l_flag | r_flag);
}

// semantic addition and subtraction for flag enum
inline flags operator-=(flags& l_flag, flags r_flag) {
  return l_flag = static_cast<flags>(l_flag & ~r_flag);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
