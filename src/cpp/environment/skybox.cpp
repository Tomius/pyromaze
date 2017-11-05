// Copyright (c) Tamas Csala

#include <lodepng.h>
#include <Silice3D/core/scene.hpp>

#include "./skybox.hpp"

Skybox::Skybox(GameObject* parent, const std::string& path)
    : GameObject(parent)
    , cube_({gl::CubeShape::kPosition})
    , prog_{GetScene()->GetShaderManager()->get("skybox.vert"),
            GetScene()->GetShaderManager()->get("skybox.frag")}
    , uProjectionMatrix_(prog_, "uProjectionMatrix")
    , uCameraMatrix_(prog_, "uCameraMatrix") {

  unsigned width, height;
  std::vector<unsigned char> data;
  unsigned error = lodepng::decode(data, width, height,
                                   path, LCT_RGBA, 8);
  if (error) {
    std::cerr << "Image decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    throw std::runtime_error("Image decoder error");
  }

  assert(width % 4 == 0);
  assert(width / 4 == height / 3);
  unsigned size = width / 4;

  gl::BindToTexUnit(texture_, Silice3D::kDiffuseTextureSlot);
  for (int i = 0; i < 6; ++i) {
    std::vector<unsigned> subdata;
    unsigned startx, starty;
    switch (i) {
      case 0: startx = 2*size; starty = 1*size; break;
      case 1: startx = 0*size; starty = 1*size; break;
      case 2: startx = 1*size; starty = 0*size; break;
      case 3: startx = 1*size; starty = 2*size; break;
      case 4: startx = 1*size; starty = 1*size; break;
      case 5: startx = 3*size; starty = 1*size; break;
    }
    for (unsigned y = starty; y < starty + size; ++y) {
      for (unsigned x = startx; x < startx + size; ++x) {
        subdata.push_back(reinterpret_cast<unsigned*>(data.data())[y*width + x]);
      }
    }
    assert(subdata.size() == size*size);
    texture_.upload(texture_.cubeFace(i), gl::kSrgb8Alpha8, size, size,
                    gl::kRgba, gl::kUnsignedByte, subdata.data());
  }
  texture_.minFilter(gl::kLinear);
  texture_.magFilter(gl::kLinear);
  gl::Unbind(texture_);

  gl::Use(prog_);
  prog_.validate();
  gl::UniformSampler(prog_, "uTex") = Silice3D::kDiffuseTextureSlot;
  (prog_ | "aPosition").bindLocation(cube_.kPosition);
  gl::Unuse(prog_);
}


void Skybox::Render() {
  gl::Use(prog_);
  prog_.Update();

  auto cam = GetScene()->GetCamera();
  uCameraMatrix_ = glm::mat3(cam->GetCameraMatrix());
  uProjectionMatrix_ = cam->GetProjectionMatrix();

  gl::TemporaryDisable depth_test{gl::kDepthTest};
  gl::TemporaryEnable cubemap_seamless{gl::kTextureCubeMapSeamless};

  gl::BindToTexUnit(texture_, Silice3D::kDiffuseTextureSlot);
  gl::DepthMask(false);

  cube_.render();

  gl::DepthMask(true);
  gl::Unbind(texture_);
  gl::Unuse(prog_);
}
