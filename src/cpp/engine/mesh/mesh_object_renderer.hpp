// Copyright (c) Tamas Csala

#ifndef MESH_OBJECT_RENDERER_HPP_
#define MESH_OBJECT_RENDERER_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/scene.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "engine/common/make_unique.hpp"

class MeshObjectRenderer : public engine::IMeshObjectRenderer {
public:
  MeshObjectRenderer (const std::string& mesh_path, engine::ShaderManager* shader_manager,
                      const std::string& vertex_shader);

  btCollisionShape* GetCollisionShape();

  void AddInstanceToRenderBatch(const engine::GameObject* game_object);
  virtual void ClearRenderBatch() override;
  virtual void RenderBatch(engine::Scene* scene) override;

  void AddInstanceToShadowRenderBatch(const engine::GameObject* game_object);
  virtual void ClearShadowRenderBatch() override;
  virtual void ShadowRenderBatch(engine::Scene* scene) override;

  engine::BoundingBox GetBoundingBox(const glm::mat4& transform) const;

  engine::ShaderProgram& basic_prog() { return basic_prog_; }
  engine::ShaderProgram& shadow_recieve_prog() { return shadow_recieve_prog_; }
  engine::ShaderProgram& shadow_cast_prog() { return shadow_cast_prog_; }

  void set_cast_shadows(bool value) { cast_shadows_ = value; }
  void set_recieve_shadows(bool value) { recieve_shadows_ = value; }

private:
  engine::MeshRenderer mesh_;

  engine::ShaderProgram basic_prog_;
  engine::ShaderProgram shadow_recieve_prog_;
  engine::ShaderProgram shadow_cast_prog_;

  // basic_prog uniforms
  gl::LazyUniform<glm::mat4> bp_uProjectionMatrix_, bp_uCameraMatrix_, bp_uModelMatrix_;

  // shadow_recieve_prog_ uniforms
  gl::LazyUniform<glm::mat4> srp_uProjectionMatrix_, srp_uCameraMatrix_, srp_uModelMatrix_, srp_uShadowCP_;

  // shadow_cast_prog_ uniforms
  gl::LazyUniform<glm::mat4> scp_uProjectionMatrix_, scp_uCameraMatrix_, scp_uModelMatrix_;

  std::vector<int> bt_indices_;
  std::unique_ptr<btTriangleIndexVertexArray> bt_triangles_;
  std::unique_ptr<btCollisionShape> bt_shape_;

  // used if (Optimizations::kDelayedModelMatrixEvalutaion)
  std::vector<const engine::GameObject*> instances_;
  std::vector<const engine::GameObject*> shadow_instances_;

  // used if (!Optimizations::kDelayedModelMatrixEvalutaion)
  std::vector<glm::mat4> instance_transforms_;
  std::vector<glm::mat4> shadow_instance_transforms_;

  bool cast_shadows_ = true;
  bool recieve_shadows_ = true;

  void EnsureModelMatrixBufferSize(size_t size);
  void SetupModelMatrixAttrib();

  std::vector<glm::mat4> ReorderTransforms(std::vector<const engine::GameObject*>& instances,
                                           const engine::ICamera& camera);
};

MeshObjectRenderer* GetMeshRenderer(const std::string& str, engine::ShaderManager* shader_manager,
                                    std::map<std::string, std::unique_ptr<engine::IMeshObjectRenderer>>* mesh_cache,
                                    const std::string& vertex_shader_path);

#endif
