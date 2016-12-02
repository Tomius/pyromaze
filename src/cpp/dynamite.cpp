// Copyright (c) Tamas Csala

#include "./dynamite.hpp"
#include "engine/scene.hpp"

class DynamiteRenderer {
 public:
  DynamiteRenderer(engine::Scene* scene)
      : mesh_("src/resource/dynamite.obj",
              aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs |
              aiProcess_PreTransformVertices)
      , prog_(scene->shader_manager()->get("dynamite.vert"),
              scene->shader_manager()->get("mesh.frag"))
      , uProjectionMatrix_(prog_, "uProjectionMatrix")
      , uCameraMatrix_(prog_, "uCameraMatrix")
      , uModelMatrix_(prog_, "uModelMatrix")
      , uNormalMatrix_(prog_, "uNormalMatrix")
      , uFirePos_(prog_, "uFirePos") {
    gl::Use(prog_);

    mesh_.setupPositions(prog_ | "aPosition");
    mesh_.setupTexCoords(prog_ | "aTexCoord");
    mesh_.setupNormals(prog_ | "aNormal");

    mesh_.setupDiffuseTextures(0);
    gl::UniformSampler(prog_, "uDiffuseTexture").set(0);

    prog_.validate();
  }

  void Render(const glm::mat4& projection_matrix,
              const glm::mat4& camera_matrix,
              const glm::mat4& model_matrix,
              const glm::mat3& normal_matrix,
              const glm::vec3& fire_pos) {
    gl::Use(prog_);
    prog_.update();

    uProjectionMatrix_ = projection_matrix;
    uCameraMatrix_ = camera_matrix;
    uModelMatrix_ = model_matrix;
    uNormalMatrix_ = normal_matrix;
    uFirePos_ = fire_pos;

    mesh_.render();
  }

 private:
  engine::MeshRenderer mesh_;
  engine::ShaderProgram prog_;

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
  gl::LazyUniform<glm::vec3> uFirePos_;
};

static DynamiteRenderer& GetDynamiteRenderer(engine::Scene* scene) {
  static DynamiteRenderer renderer{scene};
  return renderer;
}

Dynamite::Dynamite(GameObject *parent, double time_to_explode)
    : GameObject(parent)
    , spawn_time_(scene_->game_time().current_time())
    , time_to_explode_(time_to_explode) {
  fire_ = AddComponent<Fire>();
  fire_->transform().set_local_pos({0, 1.25, 0});
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
    GameObject* explosion = parent()->AddComponent<Explosion>();
    explosion->transform().set_local_pos(transform().local_pos());
    return;
  }

  for (int i = 0; i < kNumPositions - 1; ++i) {
    if (current_phase < positions[i+1].first) {
      auto& a = positions[i];
      auto& b = positions[i+1];
      glm::vec3 fire_pos = glm::mix(a.second, b.second,
                                    (current_phase-a.first)/(b.first-a.first));
      fire_->transform().set_local_pos(fire_pos);
      break;
    }
  }
}

void Dynamite::Render() {
  const auto& cam = *scene_->camera();
  GetDynamiteRenderer(scene_).Render(
      cam.projectionMatrix(),
      cam.cameraMatrix(),
      transform().matrix(),
      glm::inverse(glm::mat3(transform().matrix())),
      fire_->transform().local_pos()
  );
}
