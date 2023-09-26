#pragma once
#include <memory>

#include "rsplib/link/link.hpp"

namespace rsp {
namespace libs {
namespace job {

class job;
using job_ptr = std::shared_ptr<job>;

class job {
 public:
  // virtual void operator()(link::link_ptr link) = 0;
  virtual void run(link::link_ptr link) = 0;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
