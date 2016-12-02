#include "./main_scene.hpp"
#include "./ground.hpp"
#include "./wall.hpp"
#include "./skybox.hpp"
#include "./fire.hpp"
#include "./dynamite.hpp"
#include "./castle.hpp"

#include "common/misc.hpp"
#include "engine/camera.hpp"
#include "debug/debug_shape.hpp"
#include "debug/debug_texture.hpp"

static const glm::vec3 kLightPos = glm::normalize(glm::vec3{1.0});

MainScene::MainScene(GLFWwindow* window, engine::ShaderManager* shader_manager)
    : Scene(window, shader_manager) {
  srand(time(nullptr));
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

  AddLightSource({LightSource::Type::kDirectional, kLightPos, glm::vec3{0.05f}});

  shadow_ = AddComponent<engine::Shadow>(kLightPos, glm::vec4{0, 0, 0, 256}, 4096);
  set_shadow_camera(shadow_);
  AddComponent<Skybox>("src/resource/skybox.png");
  GameObject* castle = AddComponent<Castle>();
  castle->transform().set_scale({32, 32, 32});
  castle->transform().set_local_pos({0, -12, 0});
  AddComponent<Ground>();

  double wallLength = 0;
  for (int x = -6; x < 6; ++x) {
    for (int z = -6; z < 6; ++z) {
      Wall* wall = AddComponent<Wall>();
      if (wallLength == 0.0) {
        wallLength = wall->GetLength();
        assert(wallLength > 0);
      }
      glm::vec3 pos{x * wallLength, 0, z * wallLength};
      wall->transform().set_local_pos(pos);
    }
  }

  // AddComponent<engine::debug::DebugCube>(glm::vec3(1.0, 1.0, 0.0));

  engine::ICamera* cam = AddComponent<engine::FreeFlyCamera>(
      M_PI/3, 1, 2000, glm::vec3(16, 4, 8), glm::vec3(10, 3, 8), 15, 10);
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


void MainScene::RenderAll() {
  shadow_->Begin();
  Scene::ShadowRenderAll();
  shadow_->End();

#if 0
  DebugTexture(shader_manager()).Render(shadow_->shadow_texture());
#else
  Scene::RenderAll();
#endif
}
