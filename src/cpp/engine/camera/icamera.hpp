// Copyright (c) Tamas Csala

#ifndef ENGINE_CAMERA_ICAMERA_HPP_
#define ENGINE_CAMERA_ICAMERA_HPP_

#define GLM_FORCE_RADIANS
#include <glm/gtx/rotate_vector.hpp>

#include "engine/game_object.hpp"

namespace engine {

class ICamera : public GameObject {
public:
  using GameObject::GameObject;
  virtual glm::mat4 cameraMatrix() const = 0;
  virtual glm::mat4 projectionMatrix() const = 0;
};

}

#endif
