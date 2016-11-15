#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include "engine/scene.hpp"
#include "engine/camera.hpp"
#include "debug/debug_shape.hpp"

class MainScene : public engine::Scene {
 public:
  MainScene(GLFWwindow* window, engine::ShaderManager* shader_manager)
      : Scene(window, shader_manager) {
    #if !ENGINE_NO_FULLSCREEN
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    #endif

    AddComponent<engine::debug::DebugCube>(glm::vec3(1.0, 1.0, 0.0));

    auto cam = AddComponent<engine::FreeFlyCamera>(
        M_PI/3, 1, 500, glm::vec3(20, 5, 0), glm::vec3(), 15, 10);
    set_camera(cam);
  }

 private:
  virtual void KeyAction(int key, int scancode, int action, int mods) override {}

  virtual void Update() override {}
};

#endif
