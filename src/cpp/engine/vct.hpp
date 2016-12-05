// Copyright (c) Tamas Csala

#ifndef ENGINE_VCT_HPP_
#define ENGINE_VCT_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

namespace engine {

class VCT : public engine::GameObject {
public:
  VCT(engine::GameObject* parent);

  gl::Texture3D& voxel_texture() { return voxel_texture_; }
  gl::LazyUniform<glm::mat4>& uModelMatrix() { return uModelMatrix_; }

  void VoxelizeStart();
  void VoxelizeEnd();

  int voxel_dimensions() const { return voxel_dimensions_; }
  float voxel_grid_world_size() const { return voxel_grid_world_size_; }

private:
  engine::ShaderProgram prog_;
  gl::Texture3D voxel_texture_;
  gl::LazyUniform<glm::mat4> uShadowCP_, uModelMatrix_;

  const int voxel_dimensions_ = 128;
  const float voxel_grid_world_size_ = 512.0f;
};

}

#endif
