// Copyright (c) Tamas Csala

#include "mesh_object_renderer.hpp"
#include "settings.hpp"

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

MeshObjectRenderer::MeshObjectRenderer (const std::string& mesh_path,
                                        engine::ShaderManager* shader_manager,
                                        const std::string& vertex_shader)
    : mesh_("src/resource/" + mesh_path, aiProcessPreset_TargetRealtime_Fast |
                                         aiProcess_FlipUVs |
                                         aiProcess_PreTransformVertices |
                                         aiProcess_Triangulate |
                                         aiProcess_CalcTangentSpace)
    , basic_prog_(shader_manager->get(vertex_shader),
                  shader_manager->get("mesh.frag"))
    , shadow_recieve_prog_(shader_manager->get(vertex_shader),
                           shader_manager->get("mesh_shadow.frag"))
    , shadow_cast_prog_(shader_manager->get("shadow.vert"),
                        shader_manager->get("shadow.frag"))

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
  mesh_.setup(nullptr, nullptr, nullptr, nullptr);
  mesh_.setupDiffuseTextures(engine::kDiffuseTextureSlot);
  gl::UniformSampler(basic_prog_, "uDiffuseTexture").set(engine::kDiffuseTextureSlot);
  basic_prog_.validate();

  gl::Use(shadow_recieve_prog_);
  gl::UniformSampler(shadow_recieve_prog_, "uShadowMap").set(engine::kShadowTextureSlot);
  gl::UniformSampler(shadow_recieve_prog_, "uDiffuseTexture").set(engine::kDiffuseTextureSlot);
  shadow_recieve_prog_.validate();
  gl::Unuse(shadow_recieve_prog_);
}

btCollisionShape* MeshObjectRenderer::GetCollisionShape() {
  if (!bt_shape_) {
    bt_triangles_ = engine::make_unique<btTriangleIndexVertexArray>();
    bt_indices_ = mesh_.btTriangles(bt_triangles_.get());
    bt_shape_ = engine::make_unique<btBvhTriangleMeshShape>(bt_triangles_.get(), true);
  }

  return bt_shape_.get();
}

void MeshObjectRenderer::Render(engine::Scene* scene,
                                const engine::Transform& transform,
                                bool recieve_shadows) {
  const auto bounding_box = mesh_.boundingBox(transform.matrix());
  const auto& cam = *scene->camera();
  if (Optimizations::kFrustumCulling && !bounding_box.CollidesWithFrustum(cam.frustum())) {
    return;
  }

  if (recieve_shadows) {
    auto shadow_cam = scene->shadow_camera();

    gl::Use(shadow_recieve_prog_);
    shadow_recieve_prog_.update();

    srp_uProjectionMatrix_ = cam.projectionMatrix();
    srp_uCameraMatrix_ = cam.cameraMatrix();
    srp_uModelMatrix_ = transform.matrix();
    srp_uNormalMatrix_ = glm::inverse(glm::mat3(transform.matrix()));
    srp_uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();

    mesh_.render();
    gl::Unuse(shadow_recieve_prog_);
  } else {
    gl::Use(basic_prog_);
    basic_prog_.update();

    bp_uProjectionMatrix_ = cam.projectionMatrix();
    bp_uCameraMatrix_ = cam.cameraMatrix();
    bp_uModelMatrix_ = transform.matrix();
    bp_uNormalMatrix_ = glm::inverse(glm::mat3(transform.matrix()));

    mesh_.render();
    gl::Unuse(basic_prog_);
  }
}

void MeshObjectRenderer::ShadowRender(engine::Scene* scene,
                                      const engine::Transform& transform,
                                      bool cast_shadows) {
  if (cast_shadows) {
    const auto& shadow_cam = *scene->shadow_camera();

    gl::Use(shadow_cast_prog_);
    shadow_cast_prog_.update();

    scp_uProjectionMatrix_ = shadow_cam.projectionMatrix();
    scp_uCameraMatrix_ = shadow_cam.cameraMatrix();
    scp_uModelMatrix_ = transform.matrix();

    mesh_.render();
    gl::Unuse(shadow_cast_prog_);
  }
}

engine::BoundingBox MeshObjectRenderer::GetBoundingBox(const glm::mat4& transform) const {
  return mesh_.boundingBox(transform);
}

MeshObjectRenderer* GetMeshRenderer(const std::string& str, engine::ShaderManager* shader_manager,
                                    std::map<std::string, std::unique_ptr<engine::IMeshObjectRenderer>>* mesh_cache,
                                    const std::string& vertex_shader) {
  auto iter = mesh_cache->find(str);
  if (iter == mesh_cache->end()) {
    MeshObjectRenderer* renderer = new MeshObjectRenderer(str, shader_manager, vertex_shader);
    (*mesh_cache)[str] = std::unique_ptr<engine::IMeshObjectRenderer>{renderer};
    return renderer;
  } else {
    return dynamic_cast<MeshObjectRenderer*>(iter->second.get());
  }
}
