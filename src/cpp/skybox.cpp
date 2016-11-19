#include <lodepng.h>

#include "./skybox.hpp"
#include "engine/scene.hpp"

Skybox::Skybox(GameObject* parent, const std::string& path)
    : GameObject(parent)
    , cube_({gl::CubeShape::kPosition})
    , prog_{scene_->shader_manager()->get("skybox.vert"),
            scene_->shader_manager()->get("skybox.frag")}
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

  gl::Bind(texture_);
  for (int i = 0; i < 6; ++i) {
    std::vector<unsigned> subdata;
    int startx, starty;
    switch (i) {
      case 0: startx = 2*size; starty = 1*size; break;
      case 1: startx = 0*size; starty = 1*size; break;
      case 2: startx = 1*size; starty = 0*size; break;
      case 3: startx = 1*size; starty = 2*size; break;
      case 4: startx = 1*size; starty = 1*size; break;
      case 5: startx = 3*size; starty = 1*size; break;
    }
    for (int y = starty; y < starty + size; ++y) {
      for (int x = startx; x < startx + size; ++x) {
        subdata.push_back(reinterpret_cast<unsigned*>(data.data())[y*width + x]);
      }
    }
    assert(subdata.size() == size*size);
    texture_.upload(texture_.cubeFace(i), gl::kSrgb8Alpha8, size, size,
                    gl::kRgba, gl::kUnsignedByte, subdata.data());
  }
  texture_.minFilter(gl::kLinear);
  texture_.magFilter(gl::kLinear);

  gl::Use(prog_);
  prog_.validate();
  (prog_ | "aPosition").bindLocation(cube_.kPosition);
}


void Skybox::Render() {
  gl::Use(prog_);
  prog_.update();

  auto cam = scene_->camera();
  uCameraMatrix_ = glm::mat3(cam->cameraMatrix());
  uProjectionMatrix_ = cam->projectionMatrix();

  gl::TemporaryDisable depth_test{gl::kDepthTest};
  gl::TemporaryEnable cubemapSeamless{gl::kTextureCubeMapSeamless};

  gl::Bind(texture_);
  gl::DepthMask(false);

  cube_.render();

  gl::DepthMask(true);
  gl::Unbind(texture_);
}
