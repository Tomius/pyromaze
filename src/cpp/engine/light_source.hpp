// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_LIGHT_SOURCE_HPP_
#define ENGINE_LIGHT_SOURCE_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct LightSource {
  // directional lights are directed towards the origin
  enum class Type {kPoint, kDirectional} type;
  glm::vec3 position, color;
};

#endif
