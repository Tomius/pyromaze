#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

class MainScene : public engine::Scene {
 public:
  MainScene(GLFWwindow* window, engine::ShaderManager* shader_manager);

 private:
  engine::Shadow* shadow_;

  virtual void KeyAction(int key, int scancode, int action, int mods) override;

  virtual void Update() override;

  virtual void RenderAll() override;
};

#endif
