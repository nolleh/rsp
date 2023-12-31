/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <memory>

#include "rsplib/link/types.hpp"

namespace rsp {
namespace libs {
namespace job {

class job;
using job_ptr = std::shared_ptr<job>;

class job {
 public:
  // virtual void operator()(link::link_ptr link) = 0;
  virtual void run() = 0;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
