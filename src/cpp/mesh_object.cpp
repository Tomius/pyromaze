#include "mesh_object.hpp"

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

MeshObject::MeshObject(engine::GameObject* parent, const std::string& mesh_path,
                       const engine::Transform& initial_transform)
    : GameObject(parent, initial_transform)
    , mesh_("src/resource/" + mesh_path, aiProcessPreset_TargetRealtime_Quality |
                                         aiProcess_FlipUVs |
                                         aiProcess_PreTransformVertices)
    , basic_prog_(scene()->shader_manager()->get("mesh.vert"),
                  scene()->shader_manager()->get("mesh.frag"))
    , shadow_recieve_prog_(scene()->shader_manager()->get("mesh.vert"),
                           scene()->shader_manager()->get("mesh_shadow.frag"))
    , shadow_cast_prog_(scene()->shader_manager()->get("shadow.vert"),
                        scene()->shader_manager()->get("shadow.frag"))

    , bp_uProjectionMatrix_(basic_prog_, "uProjectionMatrix")
    , bp_uCameraMatrix_(basic_prog_, "uCameraMatrix")
    , bp_uModelMatrix_(basic_prog_, "uModelMatrix")
    , bp_uNormalMatrix_(basic_prog_, "uNormalMatrix")

    , srp_uProjectionMatrix_(shadow_recieve_prog_, "uProjectionMatrix")
    , srp_uCameraMatrix_(shadow_recieve_prog_, "uCameraMatrix")
    , srp_uModelMatrix_(shadow_recieve_prog_, "uModelMatrix")
    , srp_uShadowCP_(shadow_recieve_prog_, "uShadowCP")
    , srp_uNormalMatrix_(shadow_recieve_prog_, "uNormalMatrix")

    , scp_uProjectionMatrix_(shadow_cast_prog_, "uProjectionMatrix")
    , scp_uCameraMatrix_(shadow_cast_prog_, "uCameraMatrix")
    , scp_uModelMatrix_(shadow_cast_prog_, "uModelMatrix") {
  gl::Use(basic_prog_);
  mesh_.setupPositions(basic_prog_ | "aPosition");
  mesh_.setupTexCoords(basic_prog_ | "aTexCoord");
  mesh_.setupNormals(basic_prog_ | "aNormal");
  mesh_.setupDiffuseTextures(1);
  gl::UniformSampler(basic_prog_, "uDiffuseTexture").set(1);
  basic_prog_.validate();

  gl::Use(shadow_recieve_prog_);
  gl::UniformSampler(shadow_recieve_prog_, "uShadowMap").set(0);
  gl::UniformSampler(shadow_recieve_prog_, "uDiffuseTexture").set(1);
  shadow_recieve_prog_.validate();
}

void MeshObject::Render() {
  const auto& cam = *scene_->camera();
  if (recieve_shadows_) {
    auto& shadow_cam = *dynamic_cast<engine::Shadow*>(scene_->shadow_camera());

    gl::Use(shadow_recieve_prog_);
    shadow_recieve_prog_.update();

    gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
    shadow_cam.shadow_texture().compareMode(gl::kCompareRefToTexture);

    srp_uProjectionMatrix_ = cam.projectionMatrix();
    srp_uCameraMatrix_ = cam.cameraMatrix();
    srp_uModelMatrix_ = transform().matrix();
    srp_uNormalMatrix_ = glm::inverse(glm::mat3(transform().matrix()));
    srp_uShadowCP_ = shadow_cam.projectionMatrix() * shadow_cam.cameraMatrix();

    mesh_.render();

    gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
    shadow_cam.shadow_texture().compareMode(gl::kNone);
    gl::Unbind(shadow_cam.shadow_texture());
  } else {
    gl::Use(basic_prog_);
    basic_prog_.update();

    bp_uProjectionMatrix_ = cam.projectionMatrix();
    bp_uCameraMatrix_ = cam.cameraMatrix();
    bp_uModelMatrix_ = transform().matrix();
    bp_uNormalMatrix_ = glm::inverse(glm::mat3(transform().matrix()));

    mesh_.render();
  }
}

void MeshObject::ShadowRender() {
  if (cast_shadows_) {
    const auto& shadow_cam = *scene_->shadow_camera();

    gl::Use(shadow_cast_prog_);
    shadow_cast_prog_.update();

    scp_uProjectionMatrix_ = shadow_cam.projectionMatrix();
    scp_uCameraMatrix_ = shadow_cam.cameraMatrix();
    scp_uModelMatrix_ = transform().matrix();

    mesh_.render();
  }
}
