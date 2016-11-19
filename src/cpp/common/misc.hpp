// Copyright (c) 2016, Tamas Csala

#ifndef COMMON_MISC_HPP_
#define COMMON_MISC_HPP_

#include "common/glm.hpp"

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

#endif
