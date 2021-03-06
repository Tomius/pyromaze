// Copyright (c) Tamas Csala

#include <Silice3D/core/scene.hpp>

#include "game_logic/dynamite.hpp"

Dynamite::Dynamite(GameObject *parent,
                   const Silice3D::Transform& initial_transform,
                   double time_to_explode)
    : MeshObject(parent, "dynamite.obj", initial_transform, "dynamite.vert")
    , spawn_time_(scene_->GetGameTime().GetCurrentTime())
    , time_to_explode_(time_to_explode) {
  fire_ = AddComponent<Fire>();
  fire_->GetTransform().SetLocalPos({0, 1.25, 0});
  AddComponent<Silice3D::BulletRigidBody>(0.0f, GetCollisionShape(), Silice3D::kColStatic);
}

void Dynamite::Update() {
  MeshObject::Update();

  constexpr size_t kNumPositions = 6;
  std::pair<float, glm::vec3> positions[kNumPositions] = {
    {0, {0.52, 1.6, 0.05}},
    {0, {0.3, 1.72, 0.05}},
    {0, {0.15, 1.68, 0.05}},
    {0, {0.1, 1.62, 0.05}},
    {0, {0.01, 1.4, 0.05}},
    {0, {0.01, 1.18, 0.05}}
  };

  // make it burn with unit speed
  float sumDist = 0.0;
  for (unsigned i = 0; i < kNumPositions - 1; ++i) {
    auto& a = positions[i];
    auto& b = positions[i+1];
    sumDist += length(a.second - b.second);
  }
  float cumulativeDist = 0.0;
  for (unsigned i = 0; i < kNumPositions - 1; ++i) {
    auto& a = positions[i];
    auto& b = positions[i+1];
    cumulativeDist += length(a.second - b.second);
    b.first = cumulativeDist / sumDist;
  }

  double current_phase = (scene_->GetGameTime().GetCurrentTime() - spawn_time_) / time_to_explode_;
  if (current_phase > 1) {
    GetParent()->RemoveComponent(this);
    GameObject* explosion = GetParent()->AddComponent<Explosion>();
    explosion->GetTransform().SetLocalPos(GetTransform().GetLocalPos());
    return;
  }

  for (unsigned i = 0; i < kNumPositions - 1; ++i) {
    if (current_phase < positions[i+1].first) {
      auto& a = positions[i];
      auto& b = positions[i+1];
      glm::vec3 fire_pos = glm::mix(a.second, b.second,
                                    (current_phase-a.first)/(b.first-a.first));
      fire_->GetTransform().SetLocalPos(fire_pos);
      gl::Use(renderer_->shadow_recieve_prog());
      gl::Uniform<glm::vec3>(renderer_->shadow_recieve_prog(), "uFirePos") = fire_pos;
      gl::Unuse(renderer_->shadow_recieve_prog());
      break;
    }
  }
}
