// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_TIMER_HPP_
#define ENGINE_TIMER_HPP_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace engine {

class Timer {
 public:
  double Tick();
  void   Stop();
  void   Start();
  void   Toggle();

  double current_time() const { return current_time_; }
  double dt() const { return dt_; }

 private:
  double last_time_ = 0.0, current_time_ = 0.0, dt_ = 0.0;
  bool stopped_ = false;
};

}  // namespace engine

#endif
