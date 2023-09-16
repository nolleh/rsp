#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace utils {

std::once_flag logger::s_flag;
std::unique_ptr<logger> logger::s_instance;

logger& logger::instance() {
  std::call_once(logger::s_flag, []() {
    logger::s_instance.reset(new logger{log_level::TRACE});
  });
  return *logger::s_instance;
}

void logger::debug(std::string msg) {
  if (level > log_level::DEBUG) return;

  std::cout << msg << std::endl;
}

void logger::info(std::string msg) {
  if (level > log_level::INFO) return;
  std::cout << msg << std::endl;
}

void logger::warn(std::string msg) {
  if (level > log_level::WARN) return;

  std::cout << msg << std::endl;
}

void logger::error(std::string msg) {
  if (level > log_level::ERROR) return;

  std::cout << msg << std::endl;
}
}  // namespace utils
}  // namespace libs
}  // namespace rsp
