#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

class Player;

class MainScene : public engine::Scene {
 public:
  MainScene(engine::GameEngine* engine, GLFWwindow* window);

 private:
  engine::Shadow* shadow_;

  void CreateLabyrinth(Player* player);

  virtual void RenderAll() override;

  virtual void KeyAction(int key, int scancode, int action, int mods) override;
};

#endif
