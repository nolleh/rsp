
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
      _flags = static_cast<flags>(_flags & L_allwaysFlush);
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
      if (_flags & L_allwaysFlush) {
        *this << " |F|";
      } else if (_flags == L_startWithFlushing) {
        *this << " |SF|";
      }
      auto stream_ptr = &stream();
      s_logger* logger = this;
      do {
        *stream_ptr << "\n";
        logger = logger->mirror_stream(&stream_ptr);
      } while (stream_ptr);
      if (_flags & L_allwaysFlush || _flags == L_startWithFlushing) flush();
    }
      [[fallthrough]];
    case L_clearflags:
      if (_flags != L_startWithFlushing) {
        _flags = static_cast<flags>(_flags & L_allwaysFlush);
        _flags += _global_flags;
      }
      break;
    case L_allwaysFlush:
      _flags += L_allwaysFlush;
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
  log_date.dayNo = localTime->tm_mday;
  log_date.monthNo = localTime->tm_mon + 1;
  return localTime;
}

s_logger& s_logger::log_time() {
  *this << std::put_time(get_time(), "%d/%m/%y %H:%M:%S");
  _flags += L_time;
  return *this;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
