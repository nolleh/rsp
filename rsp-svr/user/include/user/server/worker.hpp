/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <functional>
#include <memory>

#include "rsplib/logger/logger.hpp"
#include "rsplib/thread/thread_pool.hpp"

namespace rsp {
namespace user {
namespace server {

class worker {
 public:
  using thread_pool = rsp::libs::thread_pool;
  static worker& instance() {
    std::call_once(worker::s_flag,
                   []() { worker::s_instance.reset(new worker()); });
    return *worker::s_instance;
  }

  ~worker() {
    libs::logger::logger().info() << "destroy worker" << libs::logger::L_endl;
    stop();
  }

  void start() { threads_.start(); }
  void stop() { threads_.stop(); }

  template <typename Func>
  void post(Func func) {
    threads_.post(func);
  }

  template <typename Func>
  void dispatch(Func func) {
    threads_.dispatch(func);
  }

  template <typename Func>
  auto wrap(Func func) -> decltype(std::declval<thread_pool>().wrap(func)) {
    return threads_.wrap(func);
  }

  decltype(std::declval<thread_pool>().get_executor())
  get_executor() {
    return threads_.get_executor();
  }

 private:
  // REMARK(@nolleh) not productional #. this is for conv dev
  // TODO(@nolleh) configuration feature
  worker() : threads_(30) { start(); }
  static std::once_flag s_flag;
  static std::unique_ptr<worker> s_instance;
  thread_pool threads_;
};
}  // namespace server
}  // namespace user
}  // namespace rsp
