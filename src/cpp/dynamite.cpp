// Copyright (c) 2016, Tamas Csala

#include "./dynamite.hpp"
#include "engine/scene.hpp"

Dynamite::Dynamite(GameObject *parent)
    : GameObject(parent)
    , mesh_("src/resource/dinamite.obj",
            aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs |
            aiProcess_PreTransformVertices)
    , prog_(scene_->shader_manager()->get("dynamite.vert"),
            scene_->shader_manager()->get("dynamite.frag"))
    , uProjectionMatrix_(prog_, "uProjectionMatrix")
    , uCameraMatrix_(prog_, "uCameraMatrix")
    , uModelMatrix_(prog_, "uModelMatrix")
    , uNormalMatrix_(prog_, "uNormalMatrix")
    , uFirePos_(prog_, "uFirePos")
    , spawn_time_(scene_->game_time().current_time()) {
  gl::Use(prog_);

  mesh_.setupPositions(prog_ | "aPosition");
  mesh_.setupTexCoords(prog_ | "aTexCoord");
  mesh_.setupNormals(prog_ | "aNormal");
  mesh_.setupDiffuseTextures(0);

  gl::UniformSampler(prog_, "uDiffuseTexture").set(0);

  prog_.validate();

  fire = AddComponent<Fire>();
  fire->transform().set_local_pos({0, 1.25, 0});
}

void Dynamite::Update() {
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
  for (int i = 0; i < kNumPositions - 1; ++i) {
    auto& a = positions[i];
    auto& b = positions[i+1];
    sumDist += length(a.second - b.second);
  }
  float cumulativeDist = 0.0;
  for (int i = 0; i < kNumPositions - 1; ++i) {
    auto& a = positions[i];
    auto& b = positions[i+1];
    cumulativeDist += length(a.second - b.second);
    b.first = cumulativeDist / sumDist;
  }

  double current_phase = (scene_->game_time().current_time() - spawn_time_) / time_to_explode_;
  if (current_phase > 1) {
    parent()->RemoveComponent(this);
    parent()->AddComponent<Explosion>();
    return;
  }

  for (int i = 0; i < kNumPositions - 1; ++i) {
    if (current_phase < positions[i+1].first) {
      auto& a = positions[i];
      auto& b = positions[i+1];
      glm::vec3 fire_pos = glm::mix(a.second, b.second,
                                    (current_phase-a.first)/(b.first-a.first));
      fire->transform().set_local_pos(fire_pos);
      gl::Use(prog_);
      uFirePos_ = fire_pos;
      break;
    }
  }
}

void Dynamite::Render() {
  gl::Use(prog_);
  prog_.update();

  const auto& cam = *scene_->camera();
  uCameraMatrix_ = cam.cameraMatrix();
  uProjectionMatrix_ = cam.projectionMatrix();
  uModelMatrix_ = transform().matrix();
  uNormalMatrix_ = glm::inverse(glm::mat3(transform().matrix()));

  mesh_.render();
}
