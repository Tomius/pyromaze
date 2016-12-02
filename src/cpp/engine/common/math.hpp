// Copyright (c) Tamas Csala

#ifndef COMMON_MISC_HPP_
#define COMMON_MISC_HPP_

#include "engine/common/glm.hpp"

#ifndef M_PI
  #define M_PI 3.14159265359f
#endif
#ifndef M_PI_2
  #define M_PI_2 1.57079632679f
#endif


namespace Math {

static constexpr double kEpsilon = 1e-5;

inline float Rand01() {
  return rand() / float(RAND_MAX);
}

inline glm::vec3 RandomDir() {
  float r = Rand01();
  float latitude = Rand01() * 2*M_PI;
  float longitude = Rand01() * M_PI;
  return glm::vec3{r*sin(longitude)*cos(latitude),
                   r*sin(longitude)*sin(latitude),
                   r*cos(longitude)};
}

template<typename T>
constexpr T Sqr(T v) {
  return v * v;
}

template<typename T>
constexpr T Cube(T v) {
  return v * v * v;
}

}

#endif
