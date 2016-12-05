// Copyright (c) Tamas Csala

#ifndef DEBUG_DEBUG_VOXELS_HPP_
#define DEBUG_DEBUG_VOXELS_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

#include "engine/scene.hpp"
#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

class DebugVoxels {
 public:
  DebugVoxels(engine::ShaderManager* shader_manager,
              int voxel_dimensions,
              float voxel_grid_world_size)
    : prog_{shader_manager->get("debug_voxels.vert"),
            shader_manager->get("debug_voxels.geom"),
            shader_manager->get("debug_voxels.frag")}
    , uProjectionMatrix_{prog_, "uProjectionMatrix"}
    , uCameraMatrix_{prog_, "uCameraMatrix"}
  {
    numVoxels_ = voxel_dimensions * voxel_dimensions * voxel_dimensions;
    float voxelSize = voxel_grid_world_size / voxel_dimensions;
    gl::Use(prog_);
    gl::Uniform<int>(prog_, "uDimensions") = voxel_dimensions;
    // gl::Uniform<int>(prog_, "uTotalNumVoxels") = numVoxels_;
    // gl::Uniform<float>(prog_, "uVoxelSize") = voxelSize;

    gl::Uniform<glm::mat4>(prog_, "uModelMatrix") = glm::scale(glm::vec3(voxelSize));
    gl::UniformSampler(prog_, "uVoxelsTexture") = engine::kVoxelTextureSlot;
    gl::Unuse(prog_);
  }

  void Render(engine::Scene* scene) {
    gl::Use(prog_);
    const auto& cam = *scene->camera();
    uCameraMatrix_.set(cam.cameraMatrix());
    uProjectionMatrix_.set(cam.projectionMatrix());
    gl::TemporarySet capabilities{{{gl::kBlend, true},
                                  {gl::kCullFace, false}}};
    gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);
    gl::Bind(vao_);

    gl::DrawArrays(gl::kPoints, 0, numVoxels_);

    gl::Unbind(vao_);
    gl::Unuse(prog_);
  }

 private:
  gl::VertexArray vao_;
  engine::ShaderProgram prog_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_;
  int numVoxels_;
};

#endif
