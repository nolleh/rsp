#pragma once
#include <memory>
#include <mutex>
#include <string>

#include "rsplib/logger/streamable_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

class console_logger : public s_logger {
 public:
  static console_logger &instance(log_level level = log_level::TRACE);
  s_logger *mirror_stream(ostream_ptr *mirror_stream) override {
    *mirror_stream = nullptr;
    return this;
  }

  streamable &stream() override { return ostream_; }

  s_logger& print_level() override {
    std::string represent = represent_level(_level);
    stream() << "[" << represent << "]";
    return *this;
  }

 protected:
  explicit console_logger(log_level level)
      : s_logger(L_cout, std::clog, level), ostream_(std::clog.rdbuf()) {}

  std::ostream ostream_;
  static std::once_flag s_flag;
  static std::unique_ptr<console_logger> s_instance;

 private:
  static std::once_flag s_mirror_flag;
};

}  // namespace logger
}  // namespace libs
}  // namespace rsp
