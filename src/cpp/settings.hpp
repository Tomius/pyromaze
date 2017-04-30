// Copyright (c) Tamas Csala

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

// ========================= SceneComplexity settings =========================

namespace Settings {

enum class SceneComplexity {
  kVeryLow, kLow, kMedium, kHigh, kVeryHigh, kMega, kUltra, kWtf
};

constexpr SceneComplexity kSceneComplexity = SceneComplexity::kHigh;

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

}

// ========================= Algorithmic optimizations =========================

namespace Optimizations {

constexpr bool kFrustumCulling = true;
constexpr bool kAIBugFix = true;
constexpr bool kResourceGrouping = true;
// Only effective if kResourceGrouping is true
constexpr bool kInstancing = false;
constexpr bool kSleepRobots = true;

}

/*
  +-------------------+---------+-----+--------+------+----------+-------+
  |       Setup       | VeryLow | Low | Medium | High | VeryHigh |  Mega |
  +-------------------+---------+-----+--------+------+----------+-------+
  |      Default      |   654   | 205 |   78   |  30  |    12    |  1.44 |
  +-------------------+---------+-----+--------+------+----------+-------+
  |  Frustum Culling  |   810   | 353 |  121   |  42  |    15    |  1.42 |
  +-------------------+---------+-----+--------+------+----------+-------+
  |     AI Bug Fix    |   837   | 398 |  185   |  85  |    46    |  10.7 |
  +-------------------+---------+-----+--------+------+----------+-------+
  | Resource Grouping |   870   | 683 |  484   |  275 |    155   | 11.15 |
  +-------------------+---------+-----+--------+------+----------+-------+
*/


#endif
