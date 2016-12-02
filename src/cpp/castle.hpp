#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"

class Castle : public engine::GameObject {
 public:
  Castle(engine::GameObject* parent)
      : GameObject(parent)
      , mesh_("src/resource/castle/castle.obj", aiProcessPreset_TargetRealtime_Quality |
                                               aiProcess_FlipUVs |
                                               aiProcess_PreTransformVertices)
      , prog_(scene()->shader_manager()->get("mesh.vert"),
              scene()->shader_manager()->get("mesh.frag"))
      // , shadow_prog_(scene()->shader_manager()->get("shadow.vert"),
      //                scene()->shader_manager()->get("shadow.frag"))
      , uProjectionMatrix_(prog_, "uProjectionMatrix")
      , uCameraMatrix_(prog_, "uCameraMatrix")
      // , uShadowCP_(prog_, "uShadowCP")
      , uModelMatrix_(prog_, "uModelMatrix")
      // , uSProjectionMatrix_(shadow_prog_, "uProjectionMatrix")
      // , uSCameraMatrix_(shadow_prog_, "uCameraMatrix")
      // , uSModelMatrix_(shadow_prog_, "uModelMatrix")
      , uNormalMatrix_(prog_, "uNormalMatrix") {
    gl::Use(prog_);

    mesh_.setupPositions(prog_ | "aPosition");
    mesh_.setupTexCoords(prog_ | "aTexCoord");
    mesh_.setupNormals(prog_ | "aNormal");
    mesh_.setupDiffuseTextures(0);
    // gl::UniformSampler(prog_, "uShadowMap").set(0);
    gl::UniformSampler(prog_, "uDiffuseTexture").set(0);

    prog_.validate();
  }

  virtual void Render() override {
    const auto& cam = *scene_->camera();
    // auto& shadow_cam = *dynamic_cast<engine::Shadow*>(scene_->shadow_camera());
    // gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
    // shadow_cam.shadow_texture().compareMode(gl::kCompareRefToTexture);

    gl::Use(prog_);
    prog_.update();

    uProjectionMatrix_ = cam.projectionMatrix();
    uCameraMatrix_ = cam.cameraMatrix();
    // uShadowCP_ = shadow_cam.projectionMatrix() * shadow_cam.cameraMatrix();
    uModelMatrix_ = transform().matrix();
    uNormalMatrix_ = glm::inverse(glm::mat3(transform().matrix()));

    mesh_.render();

    // gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
    // shadow_cam.shadow_texture().compareMode(gl::kNone);
    // gl::Unbind(shadow_cam.shadow_texture());
  }

  // virtual void ShadowRender() override {
  //   const auto& shadow_cam = *scene_->shadow_camera();

  //   gl::Use(shadow_prog_);
  //   shadow_prog_.update();

  //   uSProjectionMatrix_ = shadow_cam.projectionMatrix();
  //   uSCameraMatrix_ = shadow_cam.cameraMatrix();
  //   uSModelMatrix_ = transform().matrix();

  //   mesh_.render();
  // }

 private:
  engine::MeshRenderer mesh_;
  engine::ShaderProgram prog_;
  // engine::ShaderProgram shadow_prog_;

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  // gl::LazyUniform<glm::mat4> uShadowCP_, uSProjectionMatrix_, uSCameraMatrix_, uSModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
};
