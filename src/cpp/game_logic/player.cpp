// Copyright (c) Tamas Csala

#include <lodepng.h>
#include <Silice3D/core/scene.hpp>
#include <Silice3D/core/game_engine.hpp>
#include <Silice3D/debug/debug_texture.hpp>

#include "game_logic/player.hpp"
#include "game_logic/dynamite.hpp"
#include "./main_scene.hpp"

void ShowYouDiedScreen(Silice3D::ShaderManager* shader_manager) {
  unsigned width, height;
  std::vector<unsigned char> data;
  unsigned error = lodepng::decode(data, width, height, "src/resource/died.png", LCT_RGBA, 8);
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

  Silice3D::DebugTexture{shader_manager}.Render(texture);
}


Player::Player(Silice3D::GameObject* parent)
  : Silice3D::GameObject(parent)
{ }

void Player::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    Silice3D::Transform dynamite_trafo;
    if (key == GLFW_KEY_SPACE) {
      glm::dvec3 pos = transform().GetPos();
      pos += 3.0 * transform().GetForward();
      dynamite_trafo.SetPos({pos.x, 0, pos.z});
      scene()->AddComponent<Dynamite>(dynamite_trafo, 2.5 + 1.0*Silice3D::Math::Rand01());
    } else if (key == GLFW_KEY_F1) {
      for (int i = 0; i < 4; ++i) {
        dynamite_trafo.SetPos({Silice3D::Math::Rand01()*256-128, 0, Silice3D::Math::Rand01()*256-128});
        scene()->AddComponent<Dynamite>(dynamite_trafo, 2.5 + 1.0*Silice3D::Math::Rand01());
      }
    }
  }
}

void Player::ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) {
  glm::dvec3 pos = transform().GetPos();
  pos.y = 0;
  if (length(pos - exp_position) < 1.2*exp_radius) {
    ShowYouDiedScreen(scene_->shader_manager());
    glfwSwapBuffers(scene_->window());
    Silice3D::GameEngine* engine = scene_->engine();
    engine->LoadScene(std::unique_ptr<Silice3D::Scene>{new MainScene{engine}});
  }
}
