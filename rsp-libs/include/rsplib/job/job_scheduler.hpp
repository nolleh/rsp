/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#include <boost/asio.hpp>

#include "rsplib/job/job.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace job {

/**
 * has role running logic serialization.
 * */
class job_scheduler {
 public:
  ~job_scheduler() {
    auto& logger = logger::logger();
    logger.trace() << "destroy job scheduler" << logger::L_endl;
    clear();
    logger.debug() << "fin destroy job scheduler" << logger::L_endl;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(m_);
    std::queue<job_ptr> empty;
    std::swap(q_, empty);
  }

  void run() {
    auto& logger = logger::logger();

    job_ptr job = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_);
      if (q_.empty()) {
        return;
      }
      // the handler (has message, and context)
      job = q_.front();
    }

    logger.trace() << "start run job" << logger::L_endl;
    job->run();
    {
      std::lock_guard<std::mutex> lock(m_);
      q_.pop();
    }
    logger.trace() << "end run job" << logger::L_endl;
    run();
  }

  void push(job_ptr j) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(j);
  }

  void push_and_run(job_ptr j) {
    push(j);

    {
      std::lock_guard<std::mutex> lock(m_);
      if (1 != q_.size()) {
        return;
      }
    }
    run();
  }

 private:
  std::queue<job_ptr> q_;
  std::mutex m_;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
