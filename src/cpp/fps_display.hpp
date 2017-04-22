// Copyright (c), Tamas Csala

#ifndef LOD_FPS_DISPLAY_H_
#define LOD_FPS_DISPLAY_H_

#include "engine/scene.hpp"
#include "engine/game_object.hpp"

class FpsDisplay : public engine::GameObject {
 public:
  explicit FpsDisplay(engine::GameObject* parent)
      : engine::GameObject(parent), kRefreshInterval(0.5f)
      , accum_time_(scene_->camera_time().dt()) {
  }

  ~FpsDisplay() {
    if (sum_time_ != 0) {
      std::cout << "Average FPS: " << sum_frame_num_ / sum_time_ << std::endl;
    }
  }

 private:
  const float kRefreshInterval;
  double sum_frame_num_ = 0, sum_time_ = 0, accum_time_;
  int calls_ = 0;

  virtual void Update() override {
    calls_++;
    accum_time_ += scene_->camera_time().dt();
    if (accum_time_ > kRefreshInterval && accum_time_ != 0) {
      std::cout << "FPS: " << calls_ / accum_time_ << std::endl;
      sum_frame_num_ += calls_;
      sum_time_ += accum_time_;
      accum_time_ = calls_ = 0;
    }
  }
};

#endif
