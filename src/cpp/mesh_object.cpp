#include "mesh_object.hpp"

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

MeshObject::MeshObject(engine::GameObject* parent, const std::string& mesh_path,
                       const engine::Transform& initial_transform,
                       const std::string& vertex_shader)
    : GameObject(parent, initial_transform)
    , renderer_(GetMeshRenderer(mesh_path, scene_->shader_manager(), scene_->mesh_cache(), vertex_shader))
{ }

btCollisionShape* MeshObject::GetCollisionShape() {
  return renderer_->GetCollisionShape();
}

engine::BoundingBox MeshObject::GetBoundingBox() const {
  return renderer_->GetBoundingBox(transform().matrix());
}

void MeshObject::Render() {
  renderer_->Render(scene_, transform(), recieve_shadows_);
}

void MeshObject::ShadowRender() {
  renderer_->ShadowRender(scene_, transform(), cast_shadows_);
}
