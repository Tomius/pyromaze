// Copyright (c) Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "./wall.hpp"

Wall::Wall(GameObject *parent, const engine::Transform& initial_transform)
    : GameObject(parent) {
  MeshObject* pillars = AddComponent<MeshObject>("wall/pillars.obj", initial_transform);
  pillars->AddComponent<engine::BulletRigidBody>(0.0f, pillars->GetCollisionShape(), engine::kColStatic);
  pillars_bb_ = pillars->GetBoundingBox();

  for (int i = 0; i < 4; ++i) {
    if ((rand() % 4) != 0) {
      wall_parts_[i] = AddComponent<MeshObject>("wall/wall" + std::to_string(i+1) + ".obj", initial_transform);
      wall_parts_[i]->AddComponent<engine::BulletRigidBody>(0.0f, wall_parts_[i]->GetCollisionShape(), engine::kColStatic);
      walls_bb_[i] = wall_parts_[i]->GetBoundingBox();
    } else {
      wall_parts_[i] = nullptr;
    }
  }
}

engine::BoundingBox Wall::GetBoundingBox() const {
  return pillars_bb_;
}

double Wall::GetLength() const {
  return pillars_bb_.extent().x;
}

void Wall::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  for (int i = 0; i < 4; ++i) {
    if (wall_parts_[i]) {
      if (glm::length(exp_position - walls_bb_[i].center()) < exp_radius) {
        RemoveComponent(wall_parts_[i]);
        wall_parts_[i] = nullptr;
      }
    }
  }
}
