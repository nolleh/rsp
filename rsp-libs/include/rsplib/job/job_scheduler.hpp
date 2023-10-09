/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

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
  using link = link::link;

  void run() {
    auto& logger = logger::logger();

    std::function<void()> job = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_);
      if (q_.empty()) {
        return;
      }
      // the handler (has message, and context)
      job = q_.front();
    }

    logger.trace() << "start run job" << logger::L_endl;
    job();
    {
      std::lock_guard<std::mutex> lock(m_);
      q_.pop();
    }
    logger.trace() << "end run job" << logger::L_endl;
    run();
  }

  void push(job_ptr j, link* l) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(std::bind(&job::run, j, l));
  }

  void push_and_run(job_ptr j, link* l) {
    push(j, l);

    {
      std::lock_guard<std::mutex> lock(m_);
      if (1 != q_.size()) {
        return;
      }
    }
    run();
  }

  template <typename Func>
  void push_and_run(Func func) {
    {
      std::lock_guard<std::mutex> lock(m_);
      q_.push(func);

      if (1 != q_.size()) {
        return;
      }
    }
    run();
  }

 private:
  std::queue<std::function<void()>> q_;
  std::mutex m_;
};

}  // namespace job
}  // namespace libs
}  // namespace rsp
