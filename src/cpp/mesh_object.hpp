// Copyright (c) Tamas Csala

#ifndef MESH_OBJECT_HPP_
#define MESH_OBJECT_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "engine/common/make_unique.hpp"

class MeshObjectRender;

class MeshObject : public engine::GameObject {
 public:
  MeshObject(engine::GameObject* parent, const std::string& mesh_path,
             const engine::Transform& initial_transform = engine::Transform{});

  void set_cast_shadows(bool value) { cast_shadows_ = value; }
  void set_recieve_shadows(bool value) { recieve_shadows_ = value; }

  btCollisionShape* GetCollisionShape();
  engine::BoundingBox GetBoundingBox() const;

 private:
  bool cast_shadows_ = true;
  bool recieve_shadows_ = true;
  MeshObjectRender& renderer_;

  virtual void Render() override;
  virtual void Voxelize(gl::LazyUniform<glm::mat4>& uModelMatrix) override;
  virtual void ShadowRender() override;
  virtual void KeyAction(int key, int scancode, int action, int mods) override;
};

#endif
