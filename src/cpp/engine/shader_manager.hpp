// Copyright (c) Tamas Csala

#ifndef ENGINE_SHADER_MANAGER_HPP_
#define ENGINE_SHADER_MANAGER_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

namespace engine {

class ShaderFile;
class ShaderProgram;

class ShaderManager {
 public:
  ShaderFile* publish(const std::string& filename, const gl::ShaderSource& src);
  ShaderFile* get(const std::string& filename,
                  const ShaderFile* included_from = nullptr);
 private:
  std::map<std::string, std::unique_ptr<ShaderFile>> shaders_;

  template<typename... Args>
  ShaderFile* load(Args&&... args);
};

class ShaderFile : public gl::Shader {
 public:
  ShaderFile(ShaderManager& shader_manager,
             std::string filename,
             const ShaderFile* included_from = nullptr)
      : ShaderFile(shader_manager, filename,
                   gl::ShaderSource{filename}, included_from)
  { }

  ShaderFile(ShaderManager& shader_manager,
             std::string filename,
             const gl::ShaderSource& src,
             const ShaderFile* included_from = nullptr);

  void set_update_func(std::function<void(const gl::Program&)> func);

  void update(const gl::Program& prog) const;

  const std::string& exports() const { return exports_; }

 private:
  std::function<void(const gl::Program&)> update_func_;
  std::vector<ShaderFile*> includes_;
  std::string exports_;

  void findExports(std::string &src);
  void findIncludes(std::string &src, ShaderManager& shader_manager);

  friend class ShaderProgram;
};

template<typename... Args>
ShaderFile* ShaderManager::load(Args&&... args) {
  auto shader = new ShaderFile{std::forward<Args>(args)...};
  shaders_[shader->source_file_name()] = std::unique_ptr<ShaderFile>{shader};
  return shader;
}

class ShaderProgram : public gl::Program {
 public:
  ShaderProgram() {}

  template <typename... Shaders>
  explicit ShaderProgram(ShaderFile *shader, Shaders&&... shaders) {
    attachShaders(shader, shaders...);
    link();
  }

  ShaderProgram(const ShaderProgram& prog) = default;
  ShaderProgram(ShaderProgram&& prog) = default;

  void update() const;

  template<typename... Rest>
  ShaderProgram& attachShaders(ShaderFile *shader, Rest&&... rest) {
    attachShader(shader);
    attachShaders(rest...);
    return *this;
  }
  ShaderProgram& attachShaders() { return *this; }
  ShaderProgram& attachShader(ShaderFile *shader);

  virtual const Program& link() override;

 private:
  std::set<ShaderFile*> shaders_;
};

}  // namespace engine

#endif
