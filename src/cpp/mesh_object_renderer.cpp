// Copyright (c) Tamas Csala

#include "mesh_object_renderer.hpp"

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
                           shader_manager->get("mesh_vct.frag"))
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
    , srp_uCameraPos_(shadow_recieve_prog_, "uCameraPos")

    , scp_uProjectionMatrix_(shadow_cast_prog_, "uProjectionMatrix")
    , scp_uCameraMatrix_(shadow_cast_prog_, "uCameraMatrix")
    , scp_uModelMatrix_(shadow_cast_prog_, "uModelMatrix") {
  gl::Use(basic_prog_);
  mesh_.setupPositions(basic_prog_ | "aPosition");
  mesh_.setupTexCoords(basic_prog_ | "aTexCoord");
  mesh_.setupNormals(basic_prog_ | "aNormal");
  mesh_.setupTangents(gl::VertexAttrib{3});
  mesh_.setupDiffuseTextures(engine::kDiffuseTextureSlot);
  gl::UniformSampler(basic_prog_, "uDiffuseTexture").set(engine::kDiffuseTextureSlot);
  basic_prog_.validate();

  gl::Use(shadow_recieve_prog_);
  gl::UniformSampler(shadow_recieve_prog_, "uShadowMap").set(engine::kShadowTextureSlot);
  gl::UniformSampler(shadow_recieve_prog_, "uVoxelTexture").set(engine::kVoxelTextureSlot);
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
  const auto& cam = *scene->camera();
  if (recieve_shadows) {
    auto shadow_cam = scene->shadow_camera();

    gl::Use(shadow_recieve_prog_);
    shadow_recieve_prog_.update();

    srp_uProjectionMatrix_ = cam.projectionMatrix();
    srp_uCameraMatrix_ = cam.cameraMatrix();
    srp_uModelMatrix_ = transform.matrix();
    srp_uNormalMatrix_ = glm::inverse(glm::mat3(transform.matrix()));
    srp_uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();
    srp_uCameraPos_ = cam.transform().pos();

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

void MeshObjectRenderer::Voxelize() {
  mesh_.render();
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

void MeshObjectRenderer::KeyAction(int key, int scancode, int action, int mods) {
  gl::Use(shadow_recieve_prog_);
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_1:
        if (!is_pressed_[0]) {
          flags_[0] = !flags_[0];
          gl::Uniform<float>(shadow_recieve_prog_, "uShowDiffuse") = flags_[0];
        }
        is_pressed_[0] = true;
        break;
      case GLFW_KEY_2:
        if (!is_pressed_[1]) {
          flags_[1] = !flags_[1];
          gl::Uniform<float>(shadow_recieve_prog_, "uShowIndirectDiffuse") = flags_[1];
        }
        is_pressed_[1] = true;
        break;
      case GLFW_KEY_3:
        if (!is_pressed_[2]) {
          flags_[2] = !flags_[2];
          gl::Uniform<float>(shadow_recieve_prog_, "uShowIndirectSpecular") = flags_[2];
        }
        is_pressed_[2] = true;
        break;
      case GLFW_KEY_4:
        if (!is_pressed_[3]) {
          flags_[3] = !flags_[3];
          gl::Uniform<float>(shadow_recieve_prog_, "uShowAmbientOcculision") = flags_[3];
        }
        is_pressed_[3] = true;
        break;
      case GLFW_KEY_5:
        if (!is_pressed_[4]) {
          flags_[4] = !flags_[4];
          gl::Uniform<float>(shadow_recieve_prog_, "uShowAmbientOcculisionOnly") = flags_[4];
        }
        is_pressed_[4] = true;
        break;
    }
  } else if (action == GLFW_RELEASE) {
     switch (key) {
      case GLFW_KEY_1:
      is_pressed_[0] = false;
      break;
      case GLFW_KEY_2:
      is_pressed_[1] = false;
      break;
      case GLFW_KEY_3:
      is_pressed_[2] = false;
      break;
      case GLFW_KEY_4:
      is_pressed_[3] = false;
      break;
      case GLFW_KEY_5:
      is_pressed_[4] = false;
      break;
    }
  }
  gl::Unuse(shadow_recieve_prog_);
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
