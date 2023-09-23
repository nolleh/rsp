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
  L_cout = 8,
  L_tabs = 16,
  L_allwaysFlush = 32
};

// semantic addition and substraction for flag enum
inline flags operator+=(flags& l_flag, flags r_flag) {
  return l_flag = static_cast<flags>(l_flag | r_flag);
}

// semantic addition and substraction for flag enum
inline flags operator-=(flags& l_flag, flags r_flag) {
  return l_flag = static_cast<flags>(l_flag & ~r_flag);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
