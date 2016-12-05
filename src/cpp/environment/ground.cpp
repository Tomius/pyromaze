#include <lodepng.h>

#include "./ground.hpp"
#include "engine/scene.hpp"
#include "engine/shadow.hpp"
#include "engine/physics/bullet_rigid_body.hpp"

Ground::Ground(GameObject* parent)
    : GameObject(parent)
    , cube_({gl::CubeShape::kPosition,
            gl::CubeShape::kNormal,
            gl::CubeShape::kTexCoord})
    , prog_{scene_->shader_manager()->get("ground.vert"),
            scene_->shader_manager()->get("mesh_shadow.frag")}
    , uProjectionMatrix_(prog_, "uProjectionMatrix")
    , uCameraMatrix_(prog_, "uCameraMatrix")
    , uShadowCP_(prog_, "uShadowCP")
    , uModelMatrix_(prog_, "uModelMatrix")
    , render_transform_(transform()) {

  unsigned width, height;
  std::vector<unsigned char> data;
  unsigned error = lodepng::decode(data, width, height,
                                   "src/resource/ground.png", LCT_RGBA, 8);
  if (error) {
    std::cerr << "Image decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    throw std::runtime_error("Image decoder error");
  }

  gl::BindToTexUnit(texture_, engine::kDiffuseTextureSlot);
  texture_.upload(gl::kSrgb8Alpha8, width, height,
                  gl::kRgba, gl::kUnsignedByte, data.data());
  texture_.generateMipmap();
  texture_.minFilter(gl::kLinearMipmapLinear);
  texture_.magFilter(gl::kLinear);
  texture_.maxAnisotropy();
  gl::Unbind(texture_);

  gl::Use(prog_);
  prog_.validate();
  (prog_ | "aPosition").bindLocation(cube_.kPosition);
  (prog_ | "aNormal").bindLocation(cube_.kNormal);
  (prog_ | "aTexCoord").bindLocation(cube_.kTexCoord);

  gl::UniformSampler(prog_, "uShadowMap").set(engine::kShadowTextureSlot);
  gl::UniformSampler(prog_, "uDiffuseTexture").set(engine::kDiffuseTextureSlot);

  render_transform_.set_local_pos({0, -1, 0});
  render_transform_.set_scale({1024, 1, 1024});

  btCollisionShape* shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
  AddComponent<engine::BulletRigidBody>(0.0f, shape, engine::kColStatic);
  gl::Unuse(prog_);
}


void Ground::Render() {
  gl::Use(prog_);
  prog_.update();

  auto cam = scene_->camera();
  uCameraMatrix_ = cam->cameraMatrix();
  uProjectionMatrix_ = cam->projectionMatrix();
  uModelMatrix_ = render_transform_.matrix();
  auto shadow_cam = scene_->shadow_camera();
  uShadowCP_ = shadow_cam->projectionMatrix() * shadow_cam->cameraMatrix();

  gl::BindToTexUnit(texture_, engine::kDiffuseTextureSlot);
  gl::FrontFace(cube_.faceWinding());
  gl::TemporaryEnable cullface{gl::kCullFace};

  cube_.render();

  gl::UnbindFromTexUnit(texture_, engine::kDiffuseTextureSlot);

  gl::Unuse(prog_);
}
