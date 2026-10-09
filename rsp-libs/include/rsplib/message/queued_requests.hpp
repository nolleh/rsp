/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace rsp {
namespace libs {
namespace message {

// A cancellation can win only before the first authoritative mutation.
// Active tickets stay reachable until the queued operation consumes them.
// Terminal records suppress duplicates/cancel-before-request for a bounded TTL.
class queued_requests {
 public:
  using clock = std::chrono::steady_clock;
  using key = std::pair<std::string, uint64_t>;

  class ticket {
   public:
    bool try_start() {
      auto expected = state::kQueued;
      return state_.compare_exchange_strong(expected, state::kStarted);
    }
    bool started() const { return state_.load() == state::kStarted; }
    void cancel() {
      auto expected = state::kQueued;
      state_.compare_exchange_strong(expected, state::kCancelled);
    }

   private:
    enum class state { kQueued, kStarted, kCancelled };
    std::atomic<state> state_{state::kQueued};
  };

  using ticket_ptr = std::shared_ptr<ticket>;

  explicit queued_requests(
      size_t capacity = 4096,
      clock::duration retention = std::chrono::seconds(30))
      : capacity_(capacity), retention_(retention) {}

  ticket_ptr admit(const key& id, clock::time_point now = clock::now()) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(now);
    if (active_.contains(id) || terminal_.contains(id) ||
        active_.size() >= capacity_) return {};
    auto request = std::make_shared<ticket>();
    active_.emplace(id, request);
    return request;
  }

  void cancel(const key& id, clock::time_point now = clock::now()) {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(now);
    auto found = active_.find(id);
    if (found != active_.end()) {
      found->second->cancel();
    } else if (!terminal_.contains(id)) {
      remember(id, now);
    }
  }

  bool finish(const key& id, const ticket_ptr& request,
              clock::time_point now = clock::now()) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = active_.find(id);
    if (found == active_.end() || found->second != request) return false;
    active_.erase(found);
    prune(now);
    remember(id, now);
    return true;
  }

 private:
  void prune(clock::time_point now) {
    for (auto it = terminal_.begin(); it != terminal_.end();) {
      if (now >= it->second) it = terminal_.erase(it);
      else ++it;
    }
  }

  void remember(const key& id, clock::time_point now) {
    if (capacity_ == 0) return;
    if (terminal_.size() >= capacity_) {
      auto oldest = std::min_element(
          terminal_.begin(), terminal_.end(),
          [](const auto& a, const auto& b) { return a.second < b.second; });
      terminal_.erase(oldest);
    }
    terminal_[id] = now + retention_;
  }

  const size_t capacity_;
  const clock::duration retention_;
  std::mutex mutex_;
  std::map<key, ticket_ptr> active_;
  std::map<key, clock::time_point> terminal_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
