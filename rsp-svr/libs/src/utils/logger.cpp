#include "utils/logger.hpp"

namespace rsp {
namespace libs {
namespace utils {

std::once_flag logger::flag;
std::unique_ptr<logger> logger::instance;

logger& logger::get_instance() {
  std::call_once(logger::flag, []() {
    logger::instance.reset(new logger{log_level::TRACE});
  });
  return *logger::instance;
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
