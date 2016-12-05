#include "engine/vct.hpp"
#include "engine/scene.hpp"
#include "engine/game_engine.hpp"

namespace engine {

VCT::VCT(engine::GameObject* parent)
    : GameObject (parent)
    , prog_(scene_->shader_manager()->get("voxelization.vert"),
            scene_->shader_manager()->get("voxelization.geom"),
            scene_->shader_manager()->get("voxelization.frag"))
    , uShadowCP_(prog_, "uShadowCP")
    , uModelMatrix_(prog_, "uModelMatrix") {
  gl::BindToTexUnit(voxel_texture_, engine::kVoxelTextureSlot);
  voxel_texture_.minFilter(gl::kLinearMipmapLinear);
  voxel_texture_.magFilter(gl::kLinear);

  // Fill 3D texture with empty values
  int num_voxels = voxel_dimensions_ * voxel_dimensions_ * voxel_dimensions_;
  std::unique_ptr<GLubyte> data = std::unique_ptr<GLubyte>{new GLubyte[num_voxels*4]};
  std::memset(data.get(), 0, num_voxels*4);
  voxel_texture_.upload(gl::kRgba8, voxel_dimensions_, voxel_dimensions_, voxel_dimensions_, gl::kRgba, gl::kUnsignedByte, data.get());
  // voxel_texture_.upload(gl::kRgba8, voxel_dimensions_, voxel_dimensions_, voxel_dimensions_, gl::kRgba, gl::kUnsignedByte, nullptr);
  // GLubyte data[4] = {0, 0, 0, 0};
  // glClearTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  // voxel_texture_.generateMipmap();

  // Create projection matrices used to project stuff onto each axis in the voxelization step
  float size = voxel_grid_world_size_;
  // left, right, bottom, top, zNear, zFar
  glm::mat4 projectionMatrix = glm::ortho(-size*0.5f, size*0.5f, -size*0.5f, size*0.5f, size*0.5f, size*1.5f);
  gl::Use(prog_);
  gl::Uniform<glm::mat4>(prog_, "uProjX") = projectionMatrix * glm::lookAt(glm::vec3(size, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  gl::Uniform<glm::mat4>(prog_, "uProjY") = projectionMatrix * glm::lookAt(glm::vec3(0, size, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
  gl::Uniform<glm::mat4>(prog_, "uProjZ") = projectionMatrix * glm::lookAt(glm::vec3(0, 0, size), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

  gl::UniformSampler(prog_, "uShadowMap") = engine::kShadowTextureSlot;
  gl::UniformSampler(prog_, "uVoxelTexture") = engine::kVoxelTextureSlot;
  gl::UniformSampler(prog_, "uDiffuseTexture") = engine::kDiffuseTextureSlot;

  gl::Uniform<int>(prog_, "uVoxelDimensions") = voxel_dimensions_;
  gl::Unuse(prog_);
}

void VCT::VoxelizeStart() {
  gl::Use(prog_);
  prog_.update();

  auto shadow_cam = scene_->shadow_camera();
  uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();

  gl::BindToTexUnit(voxel_texture_, engine::kVoxelTextureSlot);
  // GLubyte data[4] = {0, 0, 0, 0};
  // glClearTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  int num_voxels = voxel_dimensions_ * voxel_dimensions_ * voxel_dimensions_;
  std::unique_ptr<GLubyte> data = std::unique_ptr<GLubyte>{new GLubyte[num_voxels*4]};
  std::memset(data.get(), 0, num_voxels*4);
  voxel_texture_.upload(gl::kRgba8, voxel_dimensions_, voxel_dimensions_, voxel_dimensions_, gl::kRgba, gl::kUnsignedByte, data.get());

  gl::Disable(gl::kDepthTest);
  gl::Disable(gl::kCullFace);

  gl::Viewport(0, 0, voxel_dimensions_, voxel_dimensions_);
  gl::Clear().Depth();
}

void VCT::VoxelizeEnd() {
  gl::BindToTexUnit(voxel_texture_, engine::kVoxelTextureSlot);
  voxel_texture_.generateMipmap();

  gl::Enable(gl::kDepthTest);
  gl::Unuse(prog_);
  glm::vec2 window_size = scene_->engine()->window_size();
  gl::Viewport(0, 0, window_size.x, window_size.y);
  gl::Clear().Color().Depth();
}

}

