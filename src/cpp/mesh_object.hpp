// Copyright (c) Tamas Csala

#ifndef MESH_OBJECT_HPP_
#define MESH_OBJECT_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"

class MeshObject : public engine::GameObject {
 public:
  MeshObject(engine::GameObject* parent, const std::string& mesh_path,
             const engine::Transform& initial_transform = engine::Transform{});

  void set_cast_shadows(bool value) { cast_shadows_ = value; }
  void set_recieve_shadows(bool value) { recieve_shadows_ = value; }

 private:
  bool cast_shadows_ = true;
  bool recieve_shadows_ = true;

  engine::MeshRenderer mesh_;

  engine::ShaderProgram basic_prog_;
  engine::ShaderProgram shadow_recieve_prog_;
  engine::ShaderProgram shadow_cast_prog_;

  // basic_prog uniforms
  gl::LazyUniform<glm::mat4> bp_uProjectionMatrix_, bp_uCameraMatrix_, bp_uModelMatrix_;
  gl::LazyUniform<glm::mat3> bp_uNormalMatrix_;

  // shadow_recieve_prog_ uniforms
  gl::LazyUniform<glm::mat4> srp_uProjectionMatrix_, srp_uCameraMatrix_, srp_uModelMatrix_, srp_uShadowCP_;
  gl::LazyUniform<glm::mat3> srp_uNormalMatrix_;

  // shadow_cast_prog_ uniforms
  gl::LazyUniform<glm::mat4> scp_uProjectionMatrix_, scp_uCameraMatrix_, scp_uModelMatrix_;

  virtual void Render() override;
  virtual void ShadowRender() override;
};

#endif
