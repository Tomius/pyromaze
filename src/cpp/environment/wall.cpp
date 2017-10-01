// Copyright (c) Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "./wall.hpp"

Wall::Wall(GameObject *parent, const Silice3D::Transform& initial_transform)
    : GameObject(parent) {
  Silice3D::MeshObject* pillars = AddComponent<Silice3D::MeshObject>("wall/pillars.obj", initial_transform);
  pillars->AddComponent<Silice3D::BulletRigidBody>(0.0f, pillars->GetCollisionShape(), Silice3D::kColStatic);
  pillars_bb_ = pillars->GetBoundingBox();

  for (int i = 0; i < 4; ++i) {
    if ((rand() % 4) != 0) {
      wall_parts_[i] = AddComponent<Silice3D::MeshObject>("wall/wall" + std::to_string(i+1) + ".obj", initial_transform);
      wall_parts_[i]->AddComponent<Silice3D::BulletRigidBody>(0.0f, wall_parts_[i]->GetCollisionShape(), Silice3D::kColStatic);
      walls_bb_[i] = wall_parts_[i]->GetBoundingBox();
    } else {
      wall_parts_[i] = nullptr;
    }
  }
}

Silice3D::BoundingBox Wall::GetBoundingBox() const {
  return pillars_bb_;
}

double Wall::GetLength() const {
  return pillars_bb_.extent().x;
}

void Wall::ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) {
  for (int i = 0; i < 4; ++i) {
    if (wall_parts_[i]) {
      if (glm::length(exp_position - walls_bb_[i].center()) < exp_radius) {
        RemoveComponent(wall_parts_[i]);
        wall_parts_[i] = nullptr;
      }
    }
  }
}
