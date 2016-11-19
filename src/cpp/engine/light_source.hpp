// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_LIGHT_SOURCE_HPP_
#define ENGINE_LIGHT_SOURCE_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct LightSource {
  enum class Type {kPoint, kDirectional};
  glm::vec3 poisition, direction, color;
};

#endif
