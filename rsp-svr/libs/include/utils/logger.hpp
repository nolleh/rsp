#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

namespace rsp {
namespace libs {
namespace utils {
enum class log_level { TRACE, DEBUG, INFO, WARN, ERROR };

class logger {
 public:
  static logger &get_instance();

  explicit logger(log_level level) : level(level) {}

  void trace(std::string msg);
  void debug(std::string msg);
  void info(std::string msg);
  void warn(std::string msg);
  void error(std::string msg);

 private:
  log_level level;
  static std::once_flag flag;
  static std::unique_ptr<logger> instance;
  logger(const logger &) = delete;
  logger &operator=(const logger &) = delete;
};
}  // namespace utils
}  // namespace libs
}  // namespace rsp
