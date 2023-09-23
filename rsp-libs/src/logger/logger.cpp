#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

std::once_flag logger::s_flag;
std::unique_ptr<logger> logger::s_instance;

logger& logger::instance() {
  std::call_once(logger::s_flag, []() {
    logger::s_instance.reset(new logger{log_level::TRACE});
  });
  return *logger::s_instance;
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
