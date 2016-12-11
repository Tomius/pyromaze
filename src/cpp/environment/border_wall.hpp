// Copyright (c) Tamas Csala

#ifndef ENVIRONMENT_BORDER_WALL_HPP_
#define ENVIRONMENT_BORDER_WALL_HPP_

#include <lodepng.h>

#include "./mesh_object.hpp"
#include "debug/debug_texture.hpp"
#include "game_logic/explodable.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "engine/game_engine.hpp"

class BorderWall : public MeshObject, public Explodable {
 public:
  BorderWall(engine::GameObject* parent, const std::string& path, const engine::Transform& initial_transform)
      : MeshObject(parent, path, initial_transform) {
    AddComponent<engine::BulletRigidBody>(0.0f, GetCollisionShape(), engine::kColStatic);
  }

 private:
  void ShowYouWonScreen() {
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

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override {
    if (glm::length(exp_position - transform().pos()) < 1.2*exp_radius) {
      ShowYouWonScreen();
      glfwSwapBuffers(scene_->window());
      auto eng = scene_->engine();
      eng->LoadScene(std::unique_ptr<engine::Scene>{new MainScene{eng, eng->window()}});
    }
  }
};

#endif
