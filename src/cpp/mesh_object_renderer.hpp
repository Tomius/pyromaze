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

  void Render(engine::Scene* scene,
              const engine::Transform& transform,
              bool recieve_shadows);

  void Voxelize();

  void ShadowRender(engine::Scene* scene,
                    const engine::Transform& transform,
                    bool cast_shadows);

  engine::BoundingBox GetBoundingBox(const glm::mat4& transform) const;

  void KeyAction(int key, int scancode, int action, int mods);

  engine::ShaderProgram& basic_prog() { return basic_prog_; }
  engine::ShaderProgram& shadow_recieve_prog() { return shadow_recieve_prog_; }
  engine::ShaderProgram& shadow_cast_prog() { return shadow_cast_prog_; }

private:
  engine::MeshRenderer mesh_;

  engine::ShaderProgram basic_prog_;
  engine::ShaderProgram shadow_recieve_prog_;
  engine::ShaderProgram shadow_cast_prog_;

  // basic_prog uniforms
  gl::ArrayBuffer bp_ubuffer_;
  gl::LazyUniform<glm::mat4> bp_uProjectionMatrix_, bp_uCameraMatrix_, bp_uModelMatrix_;
  gl::LazyUniform<glm::mat3> bp_uNormalMatrix_;

  // shadow_recieve_prog_ uniforms
  gl::ArrayBuffer srp_ubuffer_;
  gl::LazyUniform<glm::mat4> srp_uProjectionMatrix_, srp_uCameraMatrix_, srp_uModelMatrix_, srp_uShadowCP_;
  gl::LazyUniform<glm::mat3> srp_uNormalMatrix_;
  gl::LazyUniform<glm::vec3> srp_uCameraPos_;

  // shadow_cast_prog_ uniforms
  gl::ArrayBuffer scp_ubuffer_;
  gl::LazyUniform<glm::mat4> scp_uProjectionMatrix_, scp_uCameraMatrix_, scp_uModelMatrix_;

  bool is_pressed_[5] = {false};
  bool flags_[5] = {true, true, true, true, false};

  std::vector<int> bt_indices_;
  std::unique_ptr<btTriangleIndexVertexArray> bt_triangles_;
  std::unique_ptr<btCollisionShape> bt_shape_;
};

MeshObjectRenderer* GetMeshRenderer(const std::string& str, engine::ShaderManager* shader_manager,
                                    std::map<std::string, std::unique_ptr<engine::IMeshObjectRenderer>>* mesh_cache,
                                    const std::string& vertex_shader_path);

#endif
