#include "./main_scene.hpp"
#include "./ground.hpp"
#include "./wall.hpp"
#include "./skybox.hpp"
#include "./fire.hpp"
#include "./dynamite.hpp"

#include "common/misc.hpp"
#include "engine/camera.hpp"
#include "debug/debug_shape.hpp"

MainScene::MainScene(GLFWwindow* window, engine::ShaderManager* shader_manager)
    : Scene(window, shader_manager) {
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  shader_manager->get("lighting.frag")->set_update_func([this](const gl::Program& prog) {
    // gl::Use(prog);
    int dir_light_count = 0, pos_light_count = 0;
    for (const auto& pair : light_sources_) {
      const LightSource& light = pair.second;
      if (light.type == LightSource::Type::kPoint) {
        std::string uniform_name = "uPointLights[";
        uniform_name += std::to_string(pos_light_count++);
        uniform_name += "]";
        gl::Uniform<glm::vec3>(prog, uniform_name + ".position") = light.position;
        gl::Uniform<glm::vec3>(prog, uniform_name + ".color") = light.color;
      } else if (light.type == LightSource::Type::kDirectional) {
        std::string uniform_name = "uDirectionalLights[";
        uniform_name += std::to_string(dir_light_count++);
        uniform_name += "]";
        gl::Uniform<glm::vec3>(prog, uniform_name + ".direction") = light.position;
        gl::Uniform<glm::vec3>(prog, uniform_name + ".color") = light.color;
      }
    }
    gl::Uniform<int>(prog, "uDirectionalLightCount") = dir_light_count;
    gl::Uniform<int>(prog, "uPointLightCount") = pos_light_count;
  });

  AddLightSource({LightSource::Type::kDirectional, glm::vec3{1.0f}, glm::vec3{0.03f}});

  AddComponent<Skybox>("src/resource/skybox.png");

  AddComponent<Ground>();

  double wallLength = 0;
  for (int x = -8; x < 8; ++x) {
    for (int z = -8; z < 8; ++z) {
      Wall* wall = AddComponent<Wall>();
      if (wallLength == 0.0) {
        wallLength = wall->GetLength();
        assert(wallLength > 0);
      }
      glm::vec3 pos{x * wallLength, 0, z * wallLength};
      wall->transform().set_local_pos(pos);

      Wall* wall2 = AddComponent<Wall>();
      wall2->transform().set_rot({1, 0, 0}, {0, 0, 1});
      wall2->transform().set_local_pos(pos);
    }
  }

  // AddComponent<engine::debug::DebugCube>(glm::vec3(1.0, 1.0, 0.0));

  engine::Camera* cam = AddComponent<engine::FreeFlyCamera>(
      M_PI/3, 1, 500, glm::vec3(16, 2, 16), glm::vec3(), 15, 10);
  // engine::Camera* cam = fire->AddComponent<engine::ThirdPersonalCamera>(
  //         M_PI/3, 0.2, 500, glm::vec3(4, 3, 0));
  set_camera(cam);
}

void MainScene::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_SPACE) {
      glm::vec3 pos = scene_->camera()->transform().pos();
      pos += 5.0f * scene_->camera()->transform().forward();
      Dynamite* dynamite = AddComponent<Dynamite>(4.5 + 0.5*Rand01());
      dynamite->transform().set_local_pos({pos.x, 0, pos.z});
    } else if (key == GLFW_KEY_F1) {
      for (int i = 0; i < 4; ++i) {
        Dynamite* dynamite = AddComponent<Dynamite>(4.5 + 0.5*Rand01());
        dynamite->transform().set_local_pos({Rand01()*512-256, 0, Rand01()*512-256});
      }
    }
  }
}

void MainScene::Update() {
  glm::vec3 pos = scene_->camera()->transform().pos();
  scene_->camera()->transform().set_pos({pos.x, 4, pos.z});
}
