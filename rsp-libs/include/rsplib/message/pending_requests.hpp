/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace rsp {
namespace libs {
namespace message {

// One periodic timer for all requests sharing a channel. Callbacks run outside
// the registry lock and must dispatch application work to its serial context.
template <typename Response>
class pending_requests {
 public:
  using clock = std::chrono::steady_clock;
  using response_handler =
      std::function<void(const std::shared_ptr<Response>)>;
  using timeout_handler = std::function<void()>;

  explicit pending_requests(
      clock::duration interval = std::chrono::milliseconds(100))
      : interval_(interval) {}
  ~pending_requests() { stop(); }

  pending_requests(const pending_requests&) = delete;
  pending_requests& operator=(const pending_requests&) = delete;

  // start/stop are called by the channel owner, not from callbacks.
  void start() {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (running_) return;
    running_ = true;
    try {
      timer_ = std::thread([this] {
        std::unique_lock<std::mutex> lock(timer_mutex_);
        while (!wake_.wait_for(lock, interval_, [this] { return !running_; })) {
          lock.unlock();
          expire(clock::now());
          lock.lock();
        }
      });
    } catch (...) {
      running_ = false;
      throw;
    }
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(timer_mutex_);
      running_ = false;
    }
    wake_.notify_all();
    if (timer_.joinable()) timer_.join();
    clear();
  }

  void add(uint64_t id, response_handler response, timeout_handler timeout,
           clock::time_point deadline) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    requests_.emplace(id, request{deadline, std::move(response),
                                 std::move(timeout)});
  }

  bool cancel(uint64_t id) { return take(id).has_value(); }

  void complete(uint64_t id, const std::shared_ptr<Response>& response,
                clock::time_point now = clock::now()) {
    auto pending = take(id);
    if (!pending) return;
    // A response arriving after its deadline is late even between timer ticks.
    if (now >= pending->deadline) {
      pending->on_timeout();
    } else {
      pending->on_response(response);
    }
  }

  void expire(clock::time_point now) {
    std::vector<request> expired;
    {
      std::lock_guard<std::mutex> lock(requests_mutex_);
      for (auto it = requests_.begin(); it != requests_.end();) {
        if (now < it->second.deadline) {
          ++it;
          continue;
        }
        expired.push_back(std::move(it->second));
        it = requests_.erase(it);
      }
    }
    for (auto& pending : expired) pending.on_timeout();
  }

  void clear() {
    std::map<uint64_t, request> removed;
    {
      std::lock_guard<std::mutex> lock(requests_mutex_);
      removed.swap(requests_);
    }
  }

 private:
  struct request {
    clock::time_point deadline;
    response_handler on_response;
    timeout_handler on_timeout;
  };

  std::optional<request> take(uint64_t id) {
    std::lock_guard<std::mutex> lock(requests_mutex_);
    auto it = requests_.find(id);
    if (it == requests_.end()) return std::nullopt;
    auto removed = std::move(it->second);
    requests_.erase(it);
    return removed;
  }

  const clock::duration interval_;
  std::mutex requests_mutex_;
  std::map<uint64_t, request> requests_;
  std::mutex timer_mutex_;
  std::condition_variable wake_;
  bool running_{false};
  std::thread timer_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
