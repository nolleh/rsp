#pragma once

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <memory>

#include "rsplib/logger/logger.hpp"

namespace asio = boost::asio;
namespace rsp {
namespace libs {
namespace thread {
using logger = logger::logger;
class thread_pool {
 public:
  explicit thread_pool(size_t pool_size) : size_(pool_size) {}
  ~thread_pool() { stop(); }

  asio::io_context* io_context() { return &io_context_; }

  void start() {
    if (!thread_pool_) {
      thread_pool_ = std::make_unique<boost::asio::detail::thread_group>();
    }
    logger::instance().debug("start # of thread (" + std::to_string(size_) +
                             ")");
    // start processing loop. make post() start executing
    asio::io_context::work work(io_context_);
    for (std::size_t i = 0; i < size_; ++i) {
      // https://stackoverflow.com/questions/10555566/difference-between-c11-stdbind-and-boostbin
      thread_pool_->create_thread(
          boost::bind(&asio::io_context::run, &io_context_));
    }
  }

  void join() {
    logger::instance().debug("join for tgroup");
    thread_pool_->join();
    logger::instance().debug("end of join for tgroup");
  }

  void stop() {
    if (!thread_pool_) {
      return;
    }

    logger::instance().debug("stop # of thread(" + std::to_string(size_) + ")");
    io_context_.stop();
    thread_pool_->join();
    thread_pool_.reset();
  }

  template <typename Func>
  void post(Func func) {
    io_context_.post(func);
  }

 private:
  asio::io_context io_context_;
  boost::asio::executor_work_guard<decltype(io_context_.get_executor())>
      work_guard_{io_context_.get_executor()};
  size_t size_;
  std::unique_ptr<boost::asio::detail::thread_group> thread_pool_;
};
}  // namespace thread
}  // namespace libs
}  // namespace rsp
