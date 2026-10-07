/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <chrono>
#include <functional>
#include <memory>

#include "rsplib/link/types.hpp"

namespace rsp {
namespace libs {
namespace job {

class job;
using job_ptr = std::shared_ptr<job>;

class job {
 public:
  using completion = std::function<void()>;
  enum class cancel_reason {
    kTimeout,
    kShutdown,
  };

  virtual ~job() = default;
  virtual void run(completion done) = 0;
  virtual bool expired(std::chrono::steady_clock::time_point) const {
    return false;
  }
  virtual void cancel(cancel_reason) {}
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
