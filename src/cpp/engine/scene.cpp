// Copyright (c) Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/scene.hpp"
#include "engine/game_engine.hpp"

namespace engine {

Scene::Scene(engine::GameEngine* engine, GLFWwindow* window)
    : GameObject(nullptr)
    , camera_(nullptr)
    , window_(window)
    , engine_(engine)
    , physics_thread_should_quit_(false)
    , physics_thread_{[this](){
      while (true) {
        physics_can_run_.WaitOne();
        if (physics_thread_should_quit_) { return; }
        UpdatePhysics();
        physics_finished_.Set();
      }
    }} {
  set_scene(this);
}

Scene::~Scene() {
  ResetChildren();

  // close the physics thread
  physics_thread_should_quit_ = true;
  physics_can_run_.Set();
  physics_thread_.join();
}

ShaderManager* Scene::shader_manager() const { return engine_->shader_manager(); }

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
  physics_finished_.WaitOne();
  UpdateAll();
  physics_can_run_.Set();

  RenderAll();
  Render2DAll();
}

unsigned Scene::AddLightSource(LightSource light_source) {
  static unsigned id = 1;
  light_sources_[id] = light_source;
  return id++;
}

const LightSource& Scene::GetLightSource(unsigned id) const {
  auto iter = light_sources_.find(id);
  if (iter != light_sources_.end()) {
    return iter->second;
  } else {
    throw std::out_of_range("");
  }
}

LightSource& Scene::GetLightSource(unsigned id) {
  auto iter = light_sources_.find(id);
  if (iter != light_sources_.end()) {
    return iter->second;
  } else {
    throw std::out_of_range("");
  }
}

void Scene::EnumerateLightSources(std::function<void(const LightSource&)> processor) const {
  for (const auto& pair : light_sources_) {
    processor(pair.second);
  }
}

bool Scene::RemoveLightSource(unsigned id) {
  return light_sources_.erase(id) == 1;
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

void Scene::UpdatePhysics() {
  if (bt_world_) {
    bt_world_->stepSimulation(game_time().dt(), 16, btScalar(1.)/btScalar(120.));
  }
}

}  // namespace engine
