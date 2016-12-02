// Copyright (c) 2016, Tamas Csala

#include <string>
#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <GLFW/glfw3.h>

#include "./game_engine.hpp"

#ifdef USE_DEBUG_CONTEXT
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar *message, const void *userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    if (strcmp(message, "Program undefined behavior warning: Sampler object 0 is bound"
                        " to non-depth texture 0, yet it is used with a program that"
                        " uses a shadow sampler. This is undefined behavior.") != 0)  {
      std::cout << message << std::endl;
    }
  }
}
#endif

namespace engine {

GameEngine::GameEngine() {
  glfwSetErrorCallback(ErrorCallback);

  if (!glfwInit()) {
    std::terminate();
  }

  // Window creation
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);

#ifdef USE_DEBUG_CONTEXT
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);

#if ENGINE_NO_FULLSCREEN
  window_ = glfwCreateWindow(vidmode->width, vidmode->height,
                             "Pyromaze", nullptr, nullptr);
#else
  window_ = glfwCreateWindow(vidmode->width, vidmode->height,
                             "Pyromaze", monitor, nullptr);
#endif

  if (!window_) {
    std::cerr << "FATAL: Couldn't create a glfw window. Aborting now." << std::endl;
    glfwTerminate();
    std::terminate();
  }

  // Check the created OpenGL context's version
  int ogl_major_version = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MAJOR);
  int ogl_minor_version = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MINOR);
  std::cout << "OpenGL version: "  << ogl_major_version << '.' << ogl_minor_version << std::endl;
  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  std::cout << "Resolution: "  << width << " x " << height << std::endl;

  if (ogl_major_version < 3 || (ogl_major_version == 3 && ogl_minor_version < 3)) {
    std::cerr << "At least OpenGL version 3.3 is required to run this program\n";
    std::terminate();
  }

  glfwMakeContextCurrent(window_);

  // No V-sync needed.
  // glfwSwapInterval(0);

  bool success = gladLoadGL();
  if (!success) {
    std::cerr << "gladLoadGL failed" << std::endl;
    std::terminate();
  }

#ifdef USE_DEBUG_CONTEXT
  glDebugMessageCallback(&DebugCallback, nullptr);
#endif

  gl::Enable(gl::kDepthTest);

  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, KeyCallback);
  glfwSetCharCallback(window_, CharCallback);
  glfwSetFramebufferSizeCallback(window_, ScreenResizeCallback);
  glfwSetScrollCallback(window_, MouseScrolledCallback);
  glfwSetMouseButtonCallback(window_, MouseButtonPressed);
  glfwSetCursorPosCallback(window_, MouseMoved);
}

GameEngine::~GameEngine() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();
}

void GameEngine::LoadScene(std::unique_ptr<Scene>&& new_scene) {
  new_scene_ = std::move(new_scene);
}

glm::vec2 GameEngine::window_size() {
  if (window_) {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return glm::vec2(width, height);
  } else {
    return glm::vec2{};
  }
}

void GameEngine::Run() {
  while (!glfwWindowShouldClose(window_)) {
    if (new_scene_) {
      std::swap(scene_, new_scene_);
      new_scene_ = nullptr;
    }
    gl::Clear().Color().Depth();
    scene_->Turn();

    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

void GameEngine::ErrorCallback(int error, const char* message) {
  std::cerr << message;
}

void GameEngine::KeyCallback(GLFWwindow* window, int key, int scancode,
                             int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_F11: {
        static bool fix_mouse = false;
        fix_mouse = !fix_mouse;

        if (fix_mouse) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
      } break;
      default:
        break;
    }
  }
  GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
  if (game_engine && game_engine->scene_) {
    game_engine->scene_->KeyActionAll(key, scancode, action, mods);
  }
}

void GameEngine::CharCallback(GLFWwindow* window, unsigned codepoint) {
  GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
  if (game_engine && game_engine->scene_) {
    game_engine->scene_->CharTypedAll(codepoint);
  }
}

void GameEngine::ScreenResizeCallback(GLFWwindow* window, int width, int height) {
  gl::Viewport(width, height);
  GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
  if (game_engine && game_engine->scene_) {
    game_engine->scene_->ScreenResizedAll(width, height);
  }
}

void GameEngine::MouseScrolledCallback(GLFWwindow* window,
                                       double xoffset,
                                       double yoffset) {
  GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
  if (game_engine && game_engine->scene_) {
    game_engine->scene_->MouseScrolledAll(xoffset, yoffset);
  }
}

void GameEngine::MouseButtonPressed(GLFWwindow* window, int button,
                                    int action, int mods) {
    GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
    if (game_engine && game_engine->scene_) {
      game_engine->scene_->MouseButtonPressedAll(button, action, mods);
    }
  }

void GameEngine::MouseMoved(GLFWwindow* window, double xpos, double ypos) {
  GameEngine* game_engine = reinterpret_cast<GameEngine*>(glfwGetWindowUserPointer(window));
  if (game_engine && game_engine->scene_) {
    game_engine->scene_->MouseMovedAll(xpos, ypos);
  }
}

}  // namespace engine
