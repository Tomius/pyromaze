// Copyright (c) Tamas Csala

#ifndef CASTLE_HPP_
#define CASTLE_HPP_

#include "mesh_object.hpp"

class Castle : public MeshObject {
 public:
  Castle(engine::GameObject* parent)
      : MeshObject(parent, "castle/castle.obj") {
    set_cast_shadows(false);
    set_recieve_shadows(false);
  }
};

#endif
