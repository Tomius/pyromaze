// Copyright (c) Tamas Csala

#include "./ground.hpp"
#include "engine/physics/bullet_rigid_body.hpp"

Ground::Ground(GameObject* parent)
    : MeshObject(parent, "ground.obj") {

  AddComponent<engine::BulletRigidBody>(0.0f, GetCollisionShape(), engine::kColStatic);
}
