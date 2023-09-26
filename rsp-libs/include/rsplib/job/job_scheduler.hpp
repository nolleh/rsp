#pragma once

#include <memory>
#include <mutex>
#include <queue>

#include "rsplib/job/job.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/message/types.hpp"
namespace rsp {
namespace libs {
namespace job {

using link = rsp::libs::link::link;
using link_ptr = rsp::libs::link::link_ptr;
using job_ptr = rsp::libs::job::job_ptr;
/**
 * has role running logic serialization.
 * need thread safety logic.
 * */
class job_scheduler {
 public:
  explicit job_scheduler(link_ptr link) : link_(link) {}
  // this is run in worker thread !
  void run() {
    if (!q_.empty()) {
      return;
    }

    std::lock_guard<std::mutex> lock(m_);
    // the handler (has message, and context)
    auto job = q_.front();
    job->run(link_);
    run();
  }

  void push(job* j) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(job_ptr(j));
  }

  void push_and_run(job* j) {
    push(j);
    run();
  }

 private:
  std::queue<job_ptr> q_;
  std::mutex m_;
  link_ptr link_;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
