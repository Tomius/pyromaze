// Copyright (c) Tamas Csala

#include "engine/mesh/mesh_object_batch_renderer.hpp"

#include "engine/scene.hpp"

MeshObjectBatchRenderer::MeshObjectBatchRenderer(engine::GameObject* parent)
  : engine::GameObject(parent) { }

void MeshObjectBatchRenderer::Update() {
  engine::MeshRendererCache* cache = scene()->mesh_cache();
  for (auto& pair : *cache) {
    pair.second->ClearRenderBatch();
    pair.second->ClearShadowRenderBatch();
  }
}

void MeshObjectBatchRenderer::Render() {
  engine::MeshRendererCache* cache = scene()->mesh_cache();
  for (auto& pair : *cache) {
    pair.second->RenderBatch(scene());
  }
}

void MeshObjectBatchRenderer::ShadowRender() {
  engine::MeshRendererCache* cache = scene()->mesh_cache();
  for (auto& pair : *cache) {
    pair.second->ShadowRenderBatch(scene());
  }
}
