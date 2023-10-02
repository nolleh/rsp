#pragma once
#include <memory>
#include <mutex>

#include "rsplib/logger/streamable_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

class console_logger : public s_logger {
 public:
  static console_logger &instance();
  s_logger *mirror_stream(ostream_ptr *mirror_stream) override {
    *mirror_stream = nullptr;
    return this;
  }

  streamable &stream() override { return ostream_; }

 protected:
  explicit console_logger(log_level level)
      : s_logger(L_cout, std::clog),
        ostream_(std::clog.rdbuf()),
        level_(level) {}

  std::ostream ostream_;
  static std::once_flag s_flag;
  static std::unique_ptr<console_logger> s_instance;
  log_level level_;

 private:
  static std::once_flag s_mirror_flag;
};

}  // namespace logger
}  // namespace libs
}  // namespace rsp
