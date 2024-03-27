#ifndef __SPIN_LOCK_HPP__
#define __SPIN_LOCK_HPP__
#include <atomic>
#include <chrono>
#include <thread>

class spin_lock {
 public:
   spin_lock() = default;
  spin_lock(const spin_lock &) = delete;
  spin_lock &operator=(const spin_lock &) = delete;

  bool try_lock() {
    return flag.test_and_set(std::memory_order_acquire);
  }

  void lock(std::chrono::nanoseconds timeout) {
    auto start = std::chrono::high_resolution_clock::now();
    while (try_lock()) {
#if defined(__cpp_lib_atomic_flag_test)
      while (flag.test(std::memory_order_relaxed)) {
        if (std::chrono::high_resolution_clock::now() - start >= timeout) {
          return;
        }
        std::this_thread::yield();
      }
#endif
      if (std::chrono::high_resolution_clock::now() - start >= timeout) {
        return;
      }
      std::this_thread::yield();
    }
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }
 private:
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

#endif
