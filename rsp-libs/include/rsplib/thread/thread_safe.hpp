/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
// https://oguzhankatli.medium.com/unlocking-the-power-of-java-synchronized-blocks-in-c-8fe6fbbc0d66
#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <utility>

template <typename T>
struct auto_locker_t {
  explicit auto_locker_t(const T* p) : ptr(const_cast<T*>(p)) {
    if (ptr != 0) {
      ptr->lock();
    }
  }

  ~auto_locker_t() {
    if (ptr != 0) {
      ptr->unlock();
    }
  }
  // type cast operator overload
  operator T*() { return ptr; }

  // class member access operator overload
  T* operator->() { return ptr; }

 private:
  auto_locker_t() = delete;
  auto_locker_t(const auto_locker_t&) = delete;
  auto_locker_t& operator=(const auto_locker_t&) = delete;
  T* ptr;
};

template <typename T>
class thread_safe {
 public:
  template <typename... Args>
  explicit thread_safe(Args&&... args)
      : ptr{new lockable_T{std::forward<Args>(args)...}} {}

  // ~dtor may throw if the T class has lock somewhere else
  ~thread_safe() noexcept(false) { ptr.reset(); }

  auto operator->() { return auto_locker_t<lockable_T>(ptr.get()); }

  auto operator->() const { return auto_locker_t<lockable_T>(ptr.get()); }

 private:
  // lock/unlock methods are inserted into T class using mixin pattern
  class lockable_T : public T {
   public:
    using T::T;

    ~lockable_T() noexcept(false) {
      assert(!locked);
      if (locked) {
        throw std::runtime_error(
            "the resourse is still locked! cannot be deleted!");
      }
    }

    void lock() const {
      guard.lock();
      locked = true;
    }

    void unlock() const noexcept {
      assert(locked);
      guard.unlock();
      locked = false;
    }

   private:
    mutable std::mutex guard;
    mutable std::atomic_bool locked{false};
  };
  std::unique_ptr<lockable_T> ptr;
};
