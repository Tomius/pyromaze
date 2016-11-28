// Copyright (c) 2016, Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "./wall.hpp"
#include "engine/scene.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"
#include "engine/shadow.hpp"

class WallRenderer {
 public:
  WallRenderer(engine::Scene* scene)
      : mesh_("src/resource/old_wall.obj",
              aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs |
              aiProcess_PreTransformVertices)
      , prog_(scene->shader_manager()->get("wall.vert"),
              scene->shader_manager()->get("mesh_shadow.frag"))
      , shadow_prog_(scene->shader_manager()->get("shadow.vert"),
                     scene->shader_manager()->get("shadow.frag"))
      , uProjectionMatrix_(prog_, "uProjectionMatrix")
      , uCameraMatrix_(prog_, "uCameraMatrix")
      , uShadowCP_(prog_, "uShadowCP")
      , uModelMatrix_(prog_, "uModelMatrix")
      , uSProjectionMatrix_(shadow_prog_, "uProjectionMatrix")
      , uSCameraMatrix_(shadow_prog_, "uCameraMatrix")
      , uSModelMatrix_(shadow_prog_, "uModelMatrix")
      , uNormalMatrix_(prog_, "uNormalMatrix") {
    gl::Use(prog_);

    mesh_.setupPositions(prog_ | "aPosition");
    mesh_.setupTexCoords(prog_ | "aTexCoord");
    mesh_.setupNormals(prog_ | "aNormal");

    mesh_.setupDiffuseTextures(1);
    gl::UniformSampler(prog_, "uShadowMap").set(0);
    gl::UniformSampler(prog_, "uDiffuseTexture").set(1);

    prog_.validate();
  }

  void Render(const glm::mat4& projection_matrix,
              const glm::mat4& camera_matrix,
              const glm::mat4& shadow_cp,
              const glm::mat4& model_matrix,
              const glm::mat3& normal_matrix) {
    gl::Use(prog_);
    prog_.update();

    uProjectionMatrix_ = projection_matrix;
    uCameraMatrix_ = camera_matrix;
    uShadowCP_ = shadow_cp;
    uModelMatrix_ = model_matrix;
    uNormalMatrix_ = normal_matrix;

    mesh_.render();
  }

 void ShadowRender(const glm::mat4& projection_matrix,
                   const glm::mat4& camera_matrix,
                   const glm::mat4& model_matrix) {
    gl::Use(shadow_prog_);
    shadow_prog_.update();

    uSProjectionMatrix_ = projection_matrix;
    uSCameraMatrix_ = camera_matrix;
    uSModelMatrix_ = model_matrix;

    mesh_.render();
  }

  engine::BoundingBox GetBoundingBox(const glm::mat4& transform) const {
    return mesh_.boundingBox(transform);
  }

 private:
  engine::MeshRenderer mesh_;
  engine::ShaderProgram prog_;
  engine::ShaderProgram shadow_prog_;

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uShadowCP_, uModelMatrix_;
  gl::LazyUniform<glm::mat4> uSProjectionMatrix_, uSCameraMatrix_, uSModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
};

static WallRenderer& GetWallRenderer(engine::Scene* scene) {
  static WallRenderer renderer{scene};
  return renderer;
}

Wall::Wall(GameObject *parent) : GameObject(parent), render_transform_(&transform()) {
  render_transform_.set_local_pos({0, -0.5, 0});
  render_transform_.set_scale({60.0f / 28.1f, 60.0f / 28.1f, 60.0f / 28.1f});
}

engine::BoundingBox Wall::GetBoundingBox() const {
  return GetWallRenderer(scene_).GetBoundingBox(render_transform_.matrix());
}

double Wall::GetLength() const {
  return GetBoundingBox().extent().x;
}

void Wall::Render() {
  const auto& cam = *scene_->camera();
  auto& shadow_cam = *dynamic_cast<engine::Shadow*>(scene_->shadow_camera());
  gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
  shadow_cam.shadow_texture().compareMode(gl::kCompareRefToTexture);

  GetWallRenderer(scene_).Render(
      cam.projectionMatrix(),
      cam.cameraMatrix(),
      shadow_cam.projectionMatrix() * shadow_cam.cameraMatrix(),
      render_transform_.matrix(),
      glm::inverse(glm::mat3(render_transform_.matrix()))
  );

  gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
  shadow_cam.shadow_texture().compareMode(gl::kNone);
  gl::Unbind(shadow_cam.shadow_texture());
}

void Wall::ShadowRender() {
  const auto& shadow_cam = *scene_->shadow_camera();
  GetWallRenderer(scene_).ShadowRender(
      shadow_cam.projectionMatrix(),
      shadow_cam.cameraMatrix(),
      render_transform_.matrix()
  );
}
