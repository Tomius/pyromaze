// Copyright (c) Tamas Csala

#ifndef ENGINE_GAME_ENGINE_HPP_
#define ENGINE_GAME_ENGINE_HPP_

#include <memory>
#include "engine/scene.hpp"
#include "engine/shader_manager.hpp"

namespace engine {

class GameEngine {
 public:
  GameEngine();
  ~GameEngine();

  void LoadScene(std::unique_ptr<Scene>&& new_scene);

  Scene* scene() { return scene_.get(); }

  GLFWwindow* window() { return window_; }

  ShaderManager* shader_manager() { return &shader_manager_; }

  glm::vec2 window_size();

  void Run();

 private:
  std::unique_ptr<Scene> scene_;
  std::unique_ptr<Scene> new_scene_;
  ShaderManager shader_manager_;
  GLFWwindow *window_;

  // Callbacks
  static void ErrorCallback(int error, const char* message);

  static void KeyCallback(GLFWwindow* window, int key, int scancode,
                          int action, int mods);

  static void CharCallback(GLFWwindow* window, unsigned codepoint);

  static void ScreenResizeCallback(GLFWwindow* window, int width, int height);

  static void MouseScrolledCallback(GLFWwindow* window, double xoffset,
                                    double yoffset);

  static void MouseButtonPressed(GLFWwindow* window, int button,
                                 int action, int mods);

  static void MouseMoved(GLFWwindow* window, double xpos, double ypos);
};

}  // namespace engine

#endif
