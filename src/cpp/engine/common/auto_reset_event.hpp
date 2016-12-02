// Copyright (c) 2016, Tamas Csala

#ifndef AUTO_RESET_EVENT_H
#define AUTO_RESET_EVENT_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace engine {

class AutoResetEvent {
 public:
  explicit AutoResetEvent(bool initial = false) : flag_(initial) {}

  void Set();
  void Reset();
  bool WaitOne();

 private:
  bool flag_;
  std::mutex mutex_;
  std::condition_variable signal_;

  AutoResetEvent(const AutoResetEvent&) = delete;
  AutoResetEvent& operator=(const AutoResetEvent&) = delete;
};

}  // namespace engine

#endif
