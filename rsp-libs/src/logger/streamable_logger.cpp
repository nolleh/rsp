/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/


#include "rsplib/logger/streamable_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

s_logger& s_logger::operator<<(flags flag) {
  if (is_null()) return *this;
  if (!has_meet_level()) return *this;
  switch (flag) {
    case L_time:
      log_time();
      break;
    case L_flush:
      flags_ = static_cast<flags>(flags_ & L_allwaysFlush);
      *this << " |F|\n";
      flush();
      break;
    case L_level:
      print_level();
      add_flag(L_level);
      break;
    case L_space:
      *this << ' ';
      break;
    case L_endl: {
      if (flags_ & L_allwaysFlush) {
        *this << " |F|";
      } else if (flags_ == L_startWithFlushing) {
        *this << " |SF|";
      }
      auto stream_ptr = &stream();
      s_logger* logger = this;
      do {
        *stream_ptr << "\n";
        logger = logger->mirror_stream(&stream_ptr);
      } while (stream_ptr);
      if (flags_ & L_allwaysFlush || flags_ == L_startWithFlushing) flush();
    }
      [[fallthrough]];
    case L_clearflags:
      if (flags_ != L_startWithFlushing) {
        flags_ = static_cast<flags>(flags_ & L_allwaysFlush);
        flags_ += global_flags_;
      }
      break;
    case L_allwaysFlush:
      flags_ += L_allwaysFlush;
      break;
    case L_concat:
      remove_flag(L_tabs);
      break;
    default:
      add_flag(flag);
  }
  return *this;
}

tm* s_logger::get_time() {
  std::time_t now = std::time(nullptr);
  auto localTime = std::localtime(&now);
  log_date.day_no = localTime->tm_mday;
  log_date.month_no = localTime->tm_mon + 1;
  return localTime;
}

s_logger& s_logger::log_time() {
  *this << std::put_time(get_time(), "%d/%m/%y %H:%M:%S");
  flags_ += L_time;
  return *this;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
