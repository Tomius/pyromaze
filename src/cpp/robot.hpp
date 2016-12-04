// Copyright (c) Tamas Csala

#ifndef ROBOT_HPP_
#define ROBOT_HPP_

#include "mesh_object.hpp"

class Robot : public MeshObject {
 public:
  Robot(engine::GameObject* parent);
};

#endif
