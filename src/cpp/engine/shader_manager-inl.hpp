// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_SHADER_MANAGER_INL_HPP_
#define ENGINE_SHADER_MANAGER_INL_HPP_

#include <string>
#include "engine/shader_manager.hpp"

namespace engine {

template<typename... Args>
ShaderFile* ShaderManager::load(Args&&... args) {
  auto shader = new ShaderFile{std::forward<Args>(args)...};
  shaders_[shader->source_file()] = std::unique_ptr<ShaderFile>{shader};
  return shader;
}

template<typename... Rest>
ShaderProgram& ShaderProgram::attachShaders(ShaderFile *shader, Rest&&... rest) {
  attachShader(shader);
  attachShaders(rest...);
  return *this;
}

template <typename... Shaders>
ShaderProgram::ShaderProgram(ShaderFile *shader, Shaders&&... shaders) {
  attachShaders(shader, shaders...);
  link();
}

}  // namespace engine

#endif

