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
  static logger &instance();

  void trace(std::string msg);
  void debug(std::string msg);
  void info(std::string msg);
  void warn(std::string msg);
  void error(std::string msg);

 private:
  explicit logger(log_level level) : level(level) {}
  log_level level;
  static std::once_flag s_flag;
  static std::unique_ptr<logger> s_instance;
  logger(const logger &) = delete;
  logger &operator=(const logger &) = delete;
};
}  // namespace utils
}  // namespace libs
}  // namespace rsp
