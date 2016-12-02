#include "auto_reset_event.hpp"

namespace engine {

void AutoResetEvent::Set() {
  std::lock_guard<std::mutex> lock(mutex_);
  flag_ = true;
  signal_.notify_one();
}

void AutoResetEvent::Reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  flag_ = false;
}

bool AutoResetEvent::WaitOne() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!flag_)  // prevents false wakeups from doing harm
    signal_.wait(lock);
  flag_ = false;
  return true;
}

}  // namespace engine
