// Copyright (c) Tamas Csala

#include "./ground.hpp"
#include <Silice3D/physics/bullet_rigid_body.hpp>

Ground::Ground(GameObject* parent)
    : MeshObject(parent, "ground.obj") {

  renderer_->set_cast_shadows(false);
  AddComponent<Silice3D::BulletRigidBody>(0.0f, GetCollisionShape(), Silice3D::kColStatic);
}
