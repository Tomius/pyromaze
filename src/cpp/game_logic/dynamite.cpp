// Copyright (c) Tamas Csala

#include "game_logic/dynamite.hpp"
#include "engine/scene.hpp"

Dynamite::Dynamite(GameObject *parent,
                   const engine::Transform& initial_transform,
                   double time_to_explode)
    : MeshObject(parent, "dynamite.obj", initial_transform,
                 Optimizations::kAttribModelMat ? "dynamite_attribute_model_mat.vert"
                                                : "dynamite_uniform_model_mat.vert")
    , spawn_time_(scene_->game_time().current_time())
    , time_to_explode_(time_to_explode) {
  fire_ = AddComponent<Fire>();
  fire_->transform().set_local_pos({0, 1.25, 0});
  AddComponent<engine::BulletRigidBody>(0.0f, GetCollisionShape(), engine::kColStatic);
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

  double current_phase = (scene_->game_time().current_time() - spawn_time_) / time_to_explode_;
  if (current_phase > 1) {
    parent()->RemoveComponent(this);
    GameObject* explosion = parent()->AddComponent<Explosion>();
    explosion->transform().set_local_pos(transform().local_pos());
    return;
  }

  for (unsigned i = 0; i < kNumPositions - 1; ++i) {
    if (current_phase < positions[i+1].first) {
      auto& a = positions[i];
      auto& b = positions[i+1];
      glm::vec3 fire_pos = glm::mix(a.second, b.second,
                                    (current_phase-a.first)/(b.first-a.first));
      fire_->transform().set_local_pos(fire_pos);
      gl::Use(renderer_->shadow_recieve_prog());
      gl::Uniform<glm::vec3>(renderer_->shadow_recieve_prog(), "uFirePos") = fire_pos;
      gl::Unuse(renderer_->shadow_recieve_prog());
      break;
    }
  }

}
