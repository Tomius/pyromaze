// Copyright (c) Tamas Csala

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

// ========================= SceneComplexity settings =========================

namespace Settings {

enum class SceneComplexity {
  kVeryLow, kLow, kMedium, kHigh, kVeryHigh, kMega, kUltra, kWtf
};

constexpr SceneComplexity kSceneComplexity = SceneComplexity::kLow;

constexpr int kLabyrinthRadius =
  kSceneComplexity == SceneComplexity::kVeryLow  ? 2 :
  kSceneComplexity == SceneComplexity::kLow      ? 5 :
  kSceneComplexity == SceneComplexity::kMedium   ? 8 :
  kSceneComplexity == SceneComplexity::kHigh     ? 12 :
  kSceneComplexity == SceneComplexity::kVeryHigh ? 16 :
  kSceneComplexity == SceneComplexity::kMega     ? 32 :
  kSceneComplexity == SceneComplexity::kUltra    ? 48 :
  /*kSceneComplexity == SceneComplexity::kWtf*/    64;

constexpr int kLabyrinthDiameter = 2*(kLabyrinthRadius + 1/*border*/) + 1/*center*/;

constexpr bool kDetermininistic = true;

}

#endif
