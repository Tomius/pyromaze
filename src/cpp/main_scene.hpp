#ifndef SCENES_MAIN_SCENE_HPP_
#define SCENES_MAIN_SCENE_HPP_

#include <Silice3D/core/scene.hpp>
#include <Silice3D/lighting/shadow.hpp>

class Player;

class MainScene : public Silice3D::Scene {
 public:
  MainScene(Silice3D::GameEngine* engine, GLFWwindow* window);

 private:
  Silice3D::ICamera* player_camera_;
  Silice3D::Shadow* shadow_;

  void CreateLabyrinth(Player* player);

  virtual void RenderAll() override;

  virtual void KeyAction(int key, int scancode, int action, int mods) override;
};

#endif
