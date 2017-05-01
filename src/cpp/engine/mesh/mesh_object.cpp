#include "engine/mesh/mesh_object.hpp"
#include "settings.hpp"

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

void MeshObject::Update() {
  if (Optimizations::kResourceGrouping) {
    auto bbox = GetBoundingBox();
    const auto& cam = *scene()->camera();
    bool is_visible = !Optimizations::kFrustumCulling ||
                       bbox.CollidesWithFrustum(cam.frustum());
    if (is_visible) {
      renderer_->AddInstanceToRenderBatch(this);
    }

    const auto& shadow_cam = *scene()->shadow_camera();
    bool is_shadow_visible = !Optimizations::kFrustumCulling ||
                              bbox.CollidesWithFrustum(shadow_cam.frustum());
    if (is_shadow_visible) {
      renderer_->AddInstanceToShadowRenderBatch(this);
    }
  }
}

void MeshObject::Render() {
  if (!Optimizations::kResourceGrouping) {
    const auto& cam = *scene()->camera();
    bool is_visible = !Optimizations::kFrustumCulling ||
                       GetBoundingBox().CollidesWithFrustum(cam.frustum());
    if (is_visible) {
      renderer_->AddInstanceToRenderBatch(this);
      renderer_->RenderBatch(scene());
      renderer_->ClearRenderBatch();
    }
  }
}

void MeshObject::ShadowRender() {
  if (!Optimizations::kResourceGrouping) {
    const auto& shadow_cam = *scene()->shadow_camera();
    bool is_visible = !Optimizations::kFrustumCulling ||
                       GetBoundingBox().CollidesWithFrustum(shadow_cam.frustum());
    if (is_visible) {
      renderer_->AddInstanceToShadowRenderBatch(this);
      renderer_->ShadowRenderBatch(scene());
      renderer_->ClearShadowRenderBatch();
    }
  }
}

