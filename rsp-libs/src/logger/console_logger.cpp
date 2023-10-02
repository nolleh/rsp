
#include "rsplib/logger/console_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

std::once_flag console_logger::s_flag;
std::unique_ptr<console_logger> console_logger::s_instance;
std::once_flag console_logger::s_mirror_flag;

console_logger& console_logger::instance(log_level level) {
  std::call_once(console_logger::s_flag, [&]() {
    console_logger::s_instance.reset(new console_logger{level});
  });
  return *console_logger::s_instance;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
