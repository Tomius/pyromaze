// Copyright (c) 2016, Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "./wall.hpp"
#include "engine/scene.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"

class WallRenderer {
 public:
  WallRenderer(engine::Scene* scene)
      : mesh_("src/resource/old_wall.obj",
              aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs |
              aiProcess_PreTransformVertices)
      , prog_(scene->shader_manager()->get("wall.vert"),
              scene->shader_manager()->get("mesh.frag"))
      , uProjectionMatrix_(prog_, "uProjectionMatrix")
      , uCameraMatrix_(prog_, "uCameraMatrix")
      , uModelMatrix_(prog_, "uModelMatrix")
      , uNormalMatrix_(prog_, "uNormalMatrix") {
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
              const glm::mat3& normal_matrix) {
    gl::Use(prog_);
    prog_.update();

    uProjectionMatrix_ = projection_matrix;
    uCameraMatrix_ = camera_matrix;
    uModelMatrix_ = model_matrix;
    uNormalMatrix_ = normal_matrix;

    mesh_.render();
  }

  engine::BoundingBox GetBoundingBox(const glm::mat4& transform) const {
    return mesh_.boundingBox(transform);
  }

 private:
  engine::MeshRenderer mesh_;
  engine::ShaderProgram prog_;

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
};

static WallRenderer& GetWallRenderer(engine::Scene* scene) {
  static WallRenderer renderer{scene};
  return renderer;
}

Wall::Wall(GameObject *parent) : GameObject(parent), render_transform_(&transform()) {
  // render_transform_.set_local_pos({-3 * (GetLength() / 2 + 0.2), -0.5, -0.2});
  render_transform_.set_local_pos({0, -0.5, 0});
  render_transform_.set_scale({3, 3, 3});
}

engine::BoundingBox Wall::GetBoundingBox() const {
  return GetWallRenderer(scene_).GetBoundingBox(render_transform_.matrix());
}

double Wall::GetLength() const {
  return GetBoundingBox().extent().x;
}

void Wall::Render() {
  const auto& cam = *scene_->camera();
  GetWallRenderer(scene_).Render(
      cam.projectionMatrix(),
      cam.cameraMatrix(),
      render_transform_.matrix(),
      glm::inverse(glm::mat3(render_transform_.matrix()))
  );
}
