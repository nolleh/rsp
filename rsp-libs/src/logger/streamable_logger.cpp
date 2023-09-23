#include "rsplib/logger/streamable_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

template <typename T>
s_logger& s_logger::log(T value) {
  if (is_null()) return *this;
  auto streamPtr = &stream();
  s_logger* logger = this;
  do {
    if (is_tabs()) {
      *streamPtr << "\t";
    }
    *streamPtr << value;
    logger = logger->mirror_stream(streamPtr);
  } while (streamPtr);
  remove_flag(L_time);
  return *this;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
