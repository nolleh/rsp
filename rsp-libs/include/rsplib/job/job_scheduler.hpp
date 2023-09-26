#pragma once

#include <queue>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace job {

/**
 * has role running logic serialization
 * */
class job_scheduler {
 public:
  void run() {}

 private:
  std::queue<message::handler> q_;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
