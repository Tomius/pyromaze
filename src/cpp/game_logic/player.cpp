#include "game_logic/player.hpp"
#include "game_logic/dynamite.hpp"
#include "./main_scene.hpp"

#include "engine/scene.hpp"
#include "engine/game_engine.hpp"
#include "debug/debug_texture.hpp"

#include <lodepng.h>


void ShowYouDiedScreen(engine::ShaderManager* shader_manager) {
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

  DebugTexture{shader_manager}.Render(texture);
}


Player::Player(engine::GameObject* parent, const engine::Transform& initial_transform)
  : engine::GameObject(parent, initial_transform)
{ }

void Player::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_SPACE) {
      glm::vec3 pos = transform().pos();
      pos += 1.5f * transform().forward();
      Dynamite* dynamite = scene()->AddComponent<Dynamite>(2.5 + 1.0*Math::Rand01());
      dynamite->transform().set_pos({pos.x, 0, pos.z});
    } else if (key == GLFW_KEY_F1) {
      for (int i = 0; i < 4; ++i) {
        Dynamite* dynamite = scene()->AddComponent<Dynamite>(2.5 + 1.0*Math::Rand01());
        dynamite->transform().set_pos({Math::Rand01()*512-256, 0,
                                       Math::Rand01()*512-256});
      }
    }
  }
}

void Player::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  glm::vec3 pos = transform().pos();
  pos.y = 0;
  if (length(pos - exp_position) < 1.2f*exp_radius) {
    ShowYouDiedScreen(scene_->shader_manager());
    glfwSwapBuffers(scene_->window());
    auto eng = scene_->engine();
    eng->LoadScene(std::unique_ptr<engine::Scene>{new MainScene{eng, eng->window()}});
  }
}
