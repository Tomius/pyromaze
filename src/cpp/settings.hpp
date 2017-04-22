// Copyright (c) Tamas Csala

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

// ========================= SceneComplexity settings =========================

namespace Settings {

enum class SceneComplexity {
  kVeryLow, kLow, kMedium, kHigh, kVeryHigh, kUltra
};

constexpr SceneComplexity kSceneComplexity = SceneComplexity::kLow;

constexpr int GetLabyrinthRadius () {
  switch (kSceneComplexity) {
    case SceneComplexity::kVeryLow:  return 2;
    case SceneComplexity::kLow:      return 5;
    case SceneComplexity::kMedium:   return 8;
    case SceneComplexity::kHigh:     return 12;
    case SceneComplexity::kVeryHigh: return 16;
    case SceneComplexity::kUltra:    return 32;
  }

  return 0;
}

constexpr int kLabyrinthRadius = GetLabyrinthRadius();
constexpr int kLabyrinthDiameter = 2*(kLabyrinthRadius + 1/*border*/) + 1/*center*/;

}

// ========================= Algorithmic optimizations =========================

namespace Optimizations {

constexpr bool kFrustumCulling = true;

}

/*
    +-----------------+---------+-----+--------+------+----------+-------+
    |      Setup      | VeryLow | Low | Medium | High | VeryHigh | Ultra |
    +-----------------+---------+-----+--------+------+----------+-------+
    |     Default     |   654   | 205 |   78   |  30  |    12    |  1.44 |
    +-----------------+---------+-----+--------+------+----------+-------+
    | Frustum Culling |   683   | 346 |  106   |  41  |    16    |  1.65 |
    +-----------------+---------+-----+--------+------+----------+-------+
*/


#endif
