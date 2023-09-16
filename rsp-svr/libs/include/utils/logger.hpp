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

  template <typename Msg>
  void trace(Msg &&msg) {
    if (level > log_level::TRACE) return;

    std::cout << msg << std::endl;
  }
  template <typename Msg>
  void debug(Msg &&msg) {
    if (level > log_level::DEBUG) return;

    std::cout << msg << std::endl;
  }

  template <typename Msg>
  void info(Msg &&msg) {
    if (level > log_level::INFO) return;
    std::cout << msg << std::endl;
  }

  template <typename Msg>
  void warn(Msg &&msg) {
    if (level > log_level::WARN) return;
    std::cout << msg << std::endl;
  }

  template <typename Msg>
  void error(Msg &&msg) {
    if (level > log_level::WARN) return;
    std::cout << msg << std::endl;
  }

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
