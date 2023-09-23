#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

namespace rsp {
namespace libs {
namespace logger {
enum class log_level { TRACE, DEBUG, INFO, WARN, ERROR };

class logger {
 public:
  static logger &instance();

  template <typename Msg>
  void trace(Msg &&msg, ...) {
    if (level_ > log_level::TRACE) return;

    o_stream_ << msg << std::endl;
  }
  template <typename Msg>
  void debug(Msg &&msg, ...) {
    if (level_ > log_level::DEBUG) return;
    o_stream_ << msg << std::endl;
  }

  template <typename Msg>
  void info(Msg &&msg, ...) {
    if (level_ > log_level::INFO) return;
    o_stream_ << msg << std::endl;
  }

  template <typename Msg>
  void warn(Msg &&msg, ...) {
    if (level_ > log_level::WARN) return;
    o_stream_ << msg << std::endl;
  }

  template <typename Msg>
  void error(Msg &&msg, ...) {
    if (level_ > log_level::WARN) return;
    o_stream_ << msg << std::endl;
  }

 private:
  logger(const logger &) = delete;
  logger &operator=(const logger &) = delete;

  explicit logger(log_level level)
      : level_(level), o_stream_(std::cout.rdbuf()) {}
  log_level level_;
  static std::once_flag s_flag;
  static std::unique_ptr<logger> s_instance;
  std::ostream o_stream_;
};

}  // namespace logger
}  // namespace libs
}  // namespace rsp
