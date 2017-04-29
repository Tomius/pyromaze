// Copyright (c) Tamas Csala

#ifndef MESH_OBJECT_HPP_
#define MESH_OBJECT_HPP_

#include "mesh_object_renderer.hpp"

class MeshObject : public engine::GameObject {
 public:
  MeshObject(engine::GameObject* parent, const std::string& mesh_path,
             const engine::Transform& initial_transform = engine::Transform{},
             const std::string& vertex_shader = "mesh.vert");

  btCollisionShape* GetCollisionShape();
  engine::BoundingBox GetBoundingBox() const;
  MeshObjectRenderer* GetRenderer() const { return renderer_; }

 protected:
  MeshObjectRenderer* renderer_;

  virtual void Update() override;
  virtual void Render() override;
  virtual void ShadowRender() override;
};

#endif
