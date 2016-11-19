#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include "engine/scene.hpp"
#include "engine/camera.hpp"
#include "debug/debug_shape.hpp"
#include "./skybox.hpp"
#include "./fire.hpp"
#include "./dynamite.hpp"

class MainScene : public engine::Scene {
 public:
  MainScene(GLFWwindow* window, engine::ShaderManager* shader_manager)
      : Scene(window, shader_manager) {
    // #if !ENGINE_NO_FULLSCREEN
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // #endif

    AddComponent<Skybox>("src/resource/skybox.png");

    GameObject* ground = AddComponent<engine::debug::DebugCube>(glm::vec3(0.4, 0.4, 0.4));
    ground->transform().set_local_pos({0, -1, 0});
    ground->transform().set_scale({1024, 1, 1024});

    // AddComponent<engine::debug::DebugCube>(glm::vec3(1.0, 1.0, 0.0));

    Dynamite* dynamite = AddComponent<Dynamite>();
    dynamite->transform().set_local_pos({2, 0, 2});

    engine::Camera* cam = AddComponent<engine::FreeFlyCamera>(
        M_PI/3, 1, 500, glm::vec3(8, 4, 8), glm::vec3(), 15, 10);
    // engine::Camera* cam = fire->AddComponent<engine::ThirdPersonalCamera>(
    //         M_PI/3, 0.2, 500, glm::vec3(4, 3, 0));
    set_camera(cam);
  }

 private:
  virtual void KeyAction(int key, int scancode, int action, int mods) override {}

  virtual void Update() override {}
};

#endif
