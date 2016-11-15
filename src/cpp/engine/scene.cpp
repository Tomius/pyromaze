// Copyright (c) 2016, Tamas Csala

#include "engine/scene.hpp"
#include "engine/game_engine.hpp"

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

namespace engine {

Scene::Scene(GLFWwindow* window, ShaderManager* shader_manager)
    : GameObject(nullptr)
    , camera_(nullptr)
    , window_(window)
    , shader_manager_(shader_manager) {
  set_scene(this);
}

void Scene::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_F1:
        game_time_.Toggle();
        break;
      case GLFW_KEY_F2:
        environment_time_.Toggle();
        break;
      default:
        break;
    }
  }
}

void Scene::Turn() {
  UpdateAll();
  RenderAll();
  Render2DAll();
}

void Scene::UpdateAll() {
  game_time_.Tick();
  environment_time_.Tick();
  camera_time_.Tick();

  GameObject::UpdateAll();
}

void Scene::RenderAll() {
  if (camera_) { GameObject::RenderAll(); }
}

void Scene::Render2DAll() {
  gl::TemporarySet capabilities{{{gl::kBlend, true},
                                 {gl::kCullFace, false},
                                 {gl::kDepthTest, false}}};
  gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);

  GameObject::Render2DAll();
}

}  // namespace engine
