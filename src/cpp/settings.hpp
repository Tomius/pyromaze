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

// ========================= Optimizations =========================

namespace Optimizations {

// CPU overhead "bugs" in original code
constexpr bool kAIBugFix = true;
constexpr bool kSleepRobots = true;

// Driver overhead reduction
constexpr bool kInstanceGrouping = true;
constexpr bool kAttribModelMat = true;
constexpr bool kInvalidateBuffer = true;
constexpr bool kInstancing = true;
constexpr bool kSharedPrograms = false; /* useless */
constexpr bool kPingPongBuffer = false; /* useless */

// GPU bottleneck fixes
constexpr bool kFrustumCulling = true;

// Not so successful optimizations
constexpr bool kDelayedModelMatrixEvalutaion = false; /* performance drop */
constexpr bool kDepthOrdering = false;                /* performance drop */
constexpr bool kInverseDepthOrdering = false;         /* useless */

static_assert(!kAttribModelMat || kInstanceGrouping,
              "kAttribModelMat should only be set if kInstanceGrouping is true");
static_assert(!kInstancing || kAttribModelMat,
              "kInstancing should only be set if kAttribModelMat is true");
static_assert(!kDepthOrdering || kAttribModelMat,
              "kDepthOrdering should only be set if kAttribModelMat is true");
static_assert(!kDepthOrdering || kDelayedModelMatrixEvalutaion,
              "kDepthOrdering should only be set if kDelayedModelMatrixEvalutaion is true");
static_assert(!kInverseDepthOrdering || kDepthOrdering,
              "kInverseDepthOrdering should only be set if kDepthOrdering is true");
static_assert(!kInvalidateBuffer || kAttribModelMat,
              "kInvalidateBuffer should only be set if kAttribModelMat is true");
}




/*

Default                      -> 0.2   FPS
kAIBugFix                    -> 7.1   FPS
kFrustumCulling              -> 13    FPS
kInstanceGrouping            -> 38.27 FPS
kAttribModelMat              -> 77    FPS
-----------------------------------------
kInvalidateBuffer            -> 79    FPS
kInstancing (first try)      -> 61    FPS
kInstancing + driver update  -> 89    FPS

kDelayedModelMatrixEvalutaion   -> 67 FPS
kDepthOrdering (std::sort)      -> 15 FPS
kDepthOrdering (compute shader) -> 42 FPS

Smaller scene + Keep order      -> 330 FPS
Smaller scene + Far to close    -> 328 FPS
Smaller scene + Close to far    -> 315 FPS
*/


/*
  TODO: These FPS numbers are out of date, regenerate them at the end

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
