#pragma once

#include "logger/streamable_logger.hpp"

namespace rsp {
namespace libs {
namespace logger {

class console_logger : s_logger {
 public:
  s_logger *mirror_stream(ostream_ptr &mirror_stream) override {
    mirror_stream = &ostream_;
    return this;
  }

 protected:
  console_logger() : s_logger(L_null, std::clog), ostream_(std::clog.rdbuf()) {}
  std::ostream ostream_;
};

}  // namespace logger
}  // namespace libs
}  // namespace rsp
