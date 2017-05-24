// Copyright (c), Tamas Csala

#ifndef LOD_FPS_DISPLAY_H_
#define LOD_FPS_DISPLAY_H_

#include "statistics.hpp"
#include "engine/scene.hpp"
#include "engine/game_object.hpp"

class FpsDisplay : public engine::GameObject {
 public:
  explicit FpsDisplay(engine::GameObject* parent)
      : engine::GameObject(parent)
      , accum_time_(scene_->camera_time().dt()) {
  }

  ~FpsDisplay() {
    if (sum_time_ != 0) {
      std::cout << "Average FPS: " << sum_frame_num_ / sum_time_ << std::endl;
    }
  }

 private:
  constexpr static float kRefreshInterval = 0.25f;
  double sum_frame_num_ = 0, sum_time_ = 0, accum_time_;
  int calls_ = 0;
  bool first_display_interval_ = true;

  virtual void Update() override {
    calls_++;
    accum_time_ += scene_->camera_time().dt();
    if (accum_time_ > kRefreshInterval && accum_time_ != 0) {
      if (first_display_interval_) {
        // The first interval is usually much slower, remove that bias
        first_display_interval_ = false;
      } else {
        std::cout << "Object Count: " << Statistics::object_count << std::endl;
        std::cout << "Triangle count: " << Statistics::triangle_count << std::endl;
        std::cout << "FPS: " << calls_ / accum_time_ << std::endl << std::endl;
        sum_frame_num_ += calls_;
        sum_time_ += accum_time_;
      }

      accum_time_ = calls_ = 0;
    }
  }
};

#endif
