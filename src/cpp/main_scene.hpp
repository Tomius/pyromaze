// Copyright (c) Tamas Csala

#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include <Silice3D/core/scene.hpp>

class Player;

class MainScene : public Silice3D::Scene {
 public:
  MainScene(Silice3D::GameEngine* engine, GLFWwindow* window);

 private:
  Silice3D::GameObject* cameras_;
  Silice3D::ICamera* player_camera_;

  void CreateLabyrinth(Player* player);

  virtual void KeyAction(int key, int scancode, int action, int mods) override;
};

#endif
