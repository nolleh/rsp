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

  void push_and_run(job_ptr j) {
    job_ptr expired;
    job_ptr next;
    {
      std::lock_guard<std::mutex> lock(m_);
      q_.push(std::move(j));
      if (running_) {
        if (q_.front()->expired(std::chrono::steady_clock::now())) {
          expired = q_.front();
        }
      } else {
        running_ = true;
        next = q_.front();
      }
    }
    if (expired) {
      expired->cancel();
    } else if (next) {
      run(next);
    }
  }

 private:
  void run(const job_ptr& current) {
    logger::logger().trace() << "start run job" << logger::L_endl;
    auto completed = std::make_shared<bool>(false);
    current->run([this, completed] {
      if (*completed) return;
      *completed = true;
      complete_current();
    });
  }

  void complete_current() {
    job_ptr next;
    {
      std::lock_guard<std::mutex> lock(m_);
      if (q_.empty()) return;
      q_.pop();
      if (q_.empty()) {
        running_ = false;
        return;
      }
      next = q_.front();
    }
    logger::logger().trace() << "end run job" << logger::L_endl;
    run(next);
  }

  std::queue<job_ptr> q_;
  std::mutex m_;
  bool running_{false};
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
