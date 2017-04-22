// Copyright (c) Tamas Csala

#ifndef MESH_OBJECT_HPP_
#define MESH_OBJECT_HPP_

#include "mesh_object_renderer.hpp"

class MeshObject : public engine::GameObject {
 public:
  MeshObject(engine::GameObject* parent, const std::string& mesh_path,
             const engine::Transform& initial_transform = engine::Transform{},
             const std::string& vertex_shader = "mesh.vert");

  void set_cast_shadows(bool value) { cast_shadows_ = value; }
  void set_recieve_shadows(bool value) { recieve_shadows_ = value; }

  btCollisionShape* GetCollisionShape();
  engine::BoundingBox GetBoundingBox() const;

 protected:
  bool cast_shadows_ = true;
  bool recieve_shadows_ = true;
  MeshObjectRenderer* renderer_;

  virtual void Render() override;
  virtual void ShadowRender() override;
};

#endif
