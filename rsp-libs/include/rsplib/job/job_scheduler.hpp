#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "rsplib/job/job.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/link/types.hpp"
#include "rsplib/message/types.hpp"
namespace rsp {
namespace libs {
namespace job {

using link_ptr = link::link_ptr;

/**
 * has role running logic serialization.
 * */
class job_scheduler {
 public:
  // explicit job_scheduler(link_ptr link) : link_(link) {}
  // this is run in worker thread !
  void run() {
    if (!q_.empty()) {
      return;
    }

    std::function<void()> job = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_);
      // the handler (has message, and context)
      job = q_.front();
    }
    job();
    run();
  }

  void push(job* j, link_ptr l) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(std::bind(&job::run, job_ptr(j), l));
  }

  void push_and_run(job* j, link_ptr l) {
    push(j, l);
    run();
  }

 private:
  std::queue<std::function<void()>> q_;
  std::mutex m_;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
