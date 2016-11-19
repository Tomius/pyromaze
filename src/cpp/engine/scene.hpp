// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_SCENE_HPP_
#define ENGINE_SCENE_HPP_

#include <vector>
#include <memory>

#include "engine/timer.hpp"
#include "engine/camera.hpp"
#include "engine/game_object.hpp"
#include "engine/light_source.hpp"

namespace engine {

class ShaderManager;

class Scene : public engine::GameObject {
 public:
  Scene(GLFWwindow* window, ShaderManager* shader_manager);

  virtual float gravity() const { return 9.81f; }

  const Timer& game_time() const { return game_time_; }
  Timer& game_time() { return game_time_; }

  const Timer& environment_time() const { return environment_time_; }
  Timer& environment_time() { return environment_time_; }

  const Timer& camera_time() const { return camera_time_; }
  Timer& camera_time() { return camera_time_; }

  const Camera* camera() const { return camera_; }
  Camera* camera() { return camera_; }
  void set_camera(Camera* camera) { camera_ = camera; }

  GLFWwindow* window() const { return window_; }
  void set_window(GLFWwindow* window) { window_ = window; }

  ShaderManager* shader_manager() const { return shader_manager_; }

  virtual void Turn();

 protected:
  Camera* camera_;
  Timer game_time_, environment_time_, camera_time_;
  GLFWwindow* window_;
  ShaderManager* shader_manager_;

  virtual void KeyAction(int key, int scancode, int action, int mods) override;

  virtual void UpdateAll() override;

  virtual void RenderAll() override;

  virtual void Render2DAll() override;
};

}  // namespace engine


#endif
