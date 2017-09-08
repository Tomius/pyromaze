// Copyright (c) Tamas Csala

#include <lodepng.h>
#include <Silice3D/core/game_engine.hpp>
#include <Silice3D/physics/bullet_rigid_body.hpp>

#include "main_scene.hpp"
#include "debug/debug_texture.hpp"
#include "environment/border_wall.hpp"


BorderWall::BorderWall(Silice3D::GameObject* parent, const std::string& path, const Silice3D::Transform& initial_transform)
    : MeshObject(parent, path, initial_transform) {
  AddComponent<Silice3D::BulletRigidBody>(0.0f, GetCollisionShape(), Silice3D::kColStatic);
}

void BorderWall::ShowYouWonScreen() {
  unsigned width, height;
  std::vector<unsigned char> data;
  unsigned error = lodepng::decode(data, width, height, "src/resource/victory.png", LCT_RGBA, 8);
  if (error) {
    std::cerr << "Image decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    throw std::runtime_error("Image decoder error");
  }

  gl::Texture2D texture;
  gl::Bind(texture);
  texture.upload(gl::kSrgb8Alpha8, width, height,
                gl::kRgba, gl::kUnsignedByte, data.data());
  texture.minFilter(gl::kLinear);
  texture.magFilter(gl::kLinear);
  gl::Unbind(texture);

  DebugTexture{scene_->shader_manager()}.Render(texture);
}

void BorderWall::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  if (glm::length(exp_position - transform().pos()) < 1.2*exp_radius) {
    ShowYouWonScreen();
    glfwSwapBuffers(scene_->window());
    auto eng = scene_->engine();
    eng->LoadScene(std::unique_ptr<Silice3D::Scene>{new MainScene{eng, eng->window()}});
  }
}
