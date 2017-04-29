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

    , srp_uProjectionMatrix_(shadow_recieve_prog_, "uProjectionMatrix")
    , srp_uCameraMatrix_(shadow_recieve_prog_, "uCameraMatrix")
    , srp_uModelMatrix_(shadow_recieve_prog_, "uModelMatrix")
    , srp_uShadowCP_(shadow_recieve_prog_, "uShadowCP")

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

void MeshObjectRenderer::AddInstanceToRenderBatch(const engine::Transform& transform) {
  instance_transforms_.push_back(transform.matrix());
}

void MeshObjectRenderer::ClearRenderBatch() {
  instance_transforms_.clear();
}

void MeshObjectRenderer::RenderBatch(engine::Scene* scene) {
  const auto& cam = *scene->camera();

  mesh_.bindVao();
  gl::Bind(model_matrix_buffer_);
  // std::cout << glm::vec3(instance_transforms_[0][3]) << std::endl;
  EnsureModelMatrixBufferSize(instance_transforms_.size());
  model_matrix_buffer_.subData(0, instance_transforms_.size() * sizeof(glm::mat4),
                                  instance_transforms_.data());
  SetupModelMatrixAttrib();
  gl::Unbind(gl::kVertexArray);

  if (recieve_shadows_) {
    auto shadow_cam = scene->shadow_camera();

    gl::Use(shadow_recieve_prog_);
    shadow_recieve_prog_.update();

    srp_uProjectionMatrix_ = cam.projectionMatrix();
    srp_uCameraMatrix_ = cam.cameraMatrix();
    srp_uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();

    mesh_.render(instance_transforms_.size());

    gl::Unuse(shadow_recieve_prog_);
  } else {
    gl::Use(basic_prog_);
    basic_prog_.update();

    bp_uProjectionMatrix_ = cam.projectionMatrix();
    bp_uCameraMatrix_ = cam.cameraMatrix();

    mesh_.render(instance_transforms_.size());

    gl::Unuse(basic_prog_);
  }
}

void MeshObjectRenderer::AddInstanceToShadowRenderBatch(const engine::Transform& transform) {
  shadow_instance_transforms_.push_back(transform.matrix());
}

void MeshObjectRenderer::ClearShadowRenderBatch() {
  shadow_instance_transforms_.clear();
}

void MeshObjectRenderer::ShadowRenderBatch(engine::Scene* scene) {
  if (cast_shadows_) {
    const auto& shadow_cam = *scene->shadow_camera();

    mesh_.bindVao();
    gl::Bind(model_matrix_buffer_);
    EnsureModelMatrixBufferSize(shadow_instance_transforms_.size());
    model_matrix_buffer_.subData(0, shadow_instance_transforms_.size() * sizeof(glm::mat4),
                                    shadow_instance_transforms_.data());
    SetupModelMatrixAttrib();
    gl::Unbind(gl::kVertexArray);

    gl::Use(shadow_cast_prog_);
    shadow_cast_prog_.update();

    scp_uProjectionMatrix_ = shadow_cam.projectionMatrix();
    scp_uCameraMatrix_ = shadow_cam.cameraMatrix();

    mesh_.render(shadow_instance_transforms_.size());

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

void MeshObjectRenderer::EnsureModelMatrixBufferSize(size_t size) {
  if (model_matrix_buffer_alloc_ < size) {
    model_matrix_buffer_alloc_ = 2*size;
    model_matrix_buffer_.data(model_matrix_buffer_alloc_ * sizeof(glm::mat4), nullptr, gl::kStreamDraw);
    SetupModelMatrixAttrib();
  }
}

void MeshObjectRenderer::SetupModelMatrixAttrib() {
  for (int i = 0; i < 4; ++i) {
    auto attrib = gl::VertexAttribObject(kModelMatrixAttributeLocation + i);
    attrib.pointer(4, gl::kFloat, false, sizeof(glm::mat4), (void*)(i*sizeof(glm::vec4)));
    attrib.divisor(1);
    attrib.enable();
  }
}
