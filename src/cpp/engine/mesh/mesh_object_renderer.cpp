// Copyright (c) Tamas Csala

#include "engine/mesh/mesh_object_renderer.hpp"
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
    , shadow_cast_prog_(shader_manager->get(
               Optimizations::kAttribModelMat ? "shadow_attribute_model_mat.vert"
                                              : "shadow_uniform_model_mat.vert"),
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
  mesh_.setup();
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

static std::vector<glm::mat4> ReorderTransforms(std::vector<const engine::GameObject*>& instances,
                                                const engine::ICamera& camera) {
  std::vector<glm::mat4> transforms;
  if (Optimizations::kDepthOrdering) {
    glm::vec3 camPos = camera.transform().pos();
    std::sort(instances.begin(), instances.end(), [camPos](const engine::GameObject* a, const engine::GameObject* b) -> bool {
      float a_dist = glm::length(a->transform().pos() - camPos);
      float b_dist = glm::length(b->transform().pos() - camPos);
      return Optimizations::kInverseDepthOrdering ? a_dist > b_dist : a_dist < b_dist;
    });
  }
  for (const engine::GameObject* instance : instances) {
    transforms.push_back(instance->transform());
  }

  return transforms;
}

void MeshObjectRenderer::AddInstanceToRenderBatch(const engine::GameObject* game_object) {
  if (Optimizations::kDelayedModelMatrixEvalutaion) {
    instances_.push_back(game_object);
  } else {
    instance_transforms_.push_back(game_object->transform().matrix());
  }
}

void MeshObjectRenderer::ClearRenderBatch() {
  if (Optimizations::kDelayedModelMatrixEvalutaion) {
    instances_.clear();
  } else {
    instance_transforms_.clear();
  }
}

void MeshObjectRenderer::RenderBatch(engine::Scene* scene) {
  const auto& cam = *scene->camera();

  if (recieve_shadows_) {
    auto shadow_cam = scene->shadow_camera();

    gl::Use(shadow_recieve_prog_);
    shadow_recieve_prog_.update();

    srp_uProjectionMatrix_ = cam.projectionMatrix();
    srp_uCameraMatrix_ = cam.cameraMatrix();
    srp_uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();
  } else {
    gl::Use(basic_prog_);
    basic_prog_.update();

    bp_uProjectionMatrix_ = cam.projectionMatrix();
    bp_uCameraMatrix_ = cam.cameraMatrix();
  }

  if (Optimizations::kAttribModelMat) {
    if (Optimizations::kDelayedModelMatrixEvalutaion) {
      mesh_.uploadModelMatrices(ReorderTransforms(instances_, cam));
      mesh_.render(instances_.size());
    } else {
      mesh_.uploadModelMatrices(instance_transforms_);
      mesh_.render(instance_transforms_.size());
    }
  } else {
    for (glm::mat4& transform : instance_transforms_) {
      if (recieve_shadows_) {
        srp_uModelMatrix_ = transform;
      } else {
        bp_uModelMatrix_ = transform;
      }
      mesh_.render(1);
    }
  }
  gl::UnuseProgram();
}

void MeshObjectRenderer::AddInstanceToShadowRenderBatch(const engine::GameObject* game_object) {
  if (Optimizations::kDelayedModelMatrixEvalutaion) {
    shadow_instances_.push_back(game_object);
  } else {
    shadow_instance_transforms_.push_back(game_object->transform().matrix());
  }
}

void MeshObjectRenderer::ClearShadowRenderBatch() {
  if (Optimizations::kDelayedModelMatrixEvalutaion) {
    shadow_instances_.clear();
  } else {
    shadow_instance_transforms_.clear();
  }
}

void MeshObjectRenderer::ShadowRenderBatch(engine::Scene* scene) {
  if (cast_shadows_) {
    const auto& shadow_cam = *scene->shadow_camera();

    gl::Use(shadow_cast_prog_);
    shadow_cast_prog_.update();

    scp_uProjectionMatrix_ = shadow_cam.projectionMatrix();
    scp_uCameraMatrix_ = shadow_cam.cameraMatrix();

    if (Optimizations::kAttribModelMat) {
      if (Optimizations::kDelayedModelMatrixEvalutaion) {
        mesh_.uploadModelMatrices(ReorderTransforms(shadow_instances_, shadow_cam));
        mesh_.render(shadow_instances_.size());
      } else {
        mesh_.uploadModelMatrices(shadow_instance_transforms_);
        mesh_.render(shadow_instance_transforms_.size());
      }
    } else {
      for (glm::mat4& transform : shadow_instance_transforms_) {
        scp_uModelMatrix_ = transform;
        mesh_.render(1);
      }
    }

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
