// Copyright (c) Tamas Csala

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "./wall.hpp"
#include "engine/scene.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"
#include "engine/common/make_unique.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "engine/shadow.hpp"

#define AI_IMPORT_FLAGS aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs | aiProcess_PreTransformVertices

class WallRenderer {
 public:
  WallRenderer(engine::Scene* scene)
      : pillars_("src/resource/wall/pillars.obj", AI_IMPORT_FLAGS)
      , walls_{{"src/resource/wall/wall1.obj", AI_IMPORT_FLAGS },
                {"src/resource/wall/wall2.obj", AI_IMPORT_FLAGS },
                {"src/resource/wall/wall3.obj", AI_IMPORT_FLAGS },
                {"src/resource/wall/wall4.obj", AI_IMPORT_FLAGS }}
      , prog_(scene->shader_manager()->get("mesh.vert"),
              scene->shader_manager()->get("mesh_shadow.frag"))
      , shadow_prog_(scene->shader_manager()->get("shadow.vert"),
                     scene->shader_manager()->get("shadow.frag"))
      , uProjectionMatrix_(prog_, "uProjectionMatrix")
      , uCameraMatrix_(prog_, "uCameraMatrix")
      , uShadowCP_(prog_, "uShadowCP")
      , uModelMatrix_(prog_, "uModelMatrix")
      , uSProjectionMatrix_(shadow_prog_, "uProjectionMatrix")
      , uSCameraMatrix_(shadow_prog_, "uCameraMatrix")
      , uSModelMatrix_(shadow_prog_, "uModelMatrix")
      , uNormalMatrix_(prog_, "uNormalMatrix") {
    gl::Use(prog_);

    pillars_.setupPositions(prog_ | "aPosition");
    pillars_.setupTexCoords(prog_ | "aTexCoord");
    pillars_.setupNormals(prog_ | "aNormal");
    pillars_.setupDiffuseTextures(1);
    for (auto& wall : walls_) {
      wall.setupPositions(prog_ | "aPosition");
      wall.setupTexCoords(prog_ | "aTexCoord");
      wall.setupNormals(prog_ | "aNormal");
      wall.setupDiffuseTextures(1);
    }

    gl::UniformSampler(prog_, "uShadowMap").set(0);
    gl::UniformSampler(prog_, "uDiffuseTexture").set(1);

    prog_.validate();

    pillars_collider_.triangles = engine::make_unique<btTriangleIndexVertexArray>();
    pillars_collider_.indices = pillars_.btTriangles(pillars_collider_.triangles.get());
    pillars_collider_.shape = engine::make_unique<btBvhTriangleMeshShape>(pillars_collider_.triangles.get(), true);
    for (int i = 0; i < 4; ++i) {
      wall_colliders_[i].triangles = engine::make_unique<btTriangleIndexVertexArray>();
      wall_colliders_[i].indices = walls_[i].btTriangles(wall_colliders_[i].triangles.get());
      wall_colliders_[i].shape = engine::make_unique<btBvhTriangleMeshShape>(wall_colliders_[i].triangles.get(), true);
    }
  }

  void Render(const glm::mat4& projection_matrix,
              const glm::mat4& camera_matrix,
              const glm::mat4& shadow_cp,
              const glm::mat4& model_matrix,
              const glm::mat3& normal_matrix,
              const std::array<bool, 4>& wall_parts_up) {
    gl::Use(prog_);
    prog_.update();

    uProjectionMatrix_ = projection_matrix;
    uCameraMatrix_ = camera_matrix;
    uShadowCP_ = shadow_cp;
    uModelMatrix_ = model_matrix;
    uNormalMatrix_ = normal_matrix;

    pillars_.render();
    for (int i = 0; i < 4; ++i) {
      if (wall_parts_up[i]) {
        walls_[i].render();
      }
    }
  }

 void ShadowRender(const glm::mat4& projection_matrix,
                   const glm::mat4& camera_matrix,
                   const glm::mat4& model_matrix,
                   const std::array<bool, 4>& wall_parts_up) {
    gl::Use(shadow_prog_);
    shadow_prog_.update();

    uSProjectionMatrix_ = projection_matrix;
    uSCameraMatrix_ = camera_matrix;
    uSModelMatrix_ = model_matrix;

    pillars_.render();
    for (int i = 0; i < 4; ++i) {
      if (wall_parts_up[i]) {
        walls_[i].render();
      }
    }
  }

  engine::BoundingBox GetWallBoundingBox(const glm::mat4& transform, int wall_index) const {
    if (wall_index < 0 || 4 <= wall_index) {
      throw std::out_of_range("");
    }
    return walls_[wall_index].boundingBox(transform);
  }

  btCollisionShape* GetWallCollisionShape(int wall_index) {
    if (wall_index < 0 || 4 <= wall_index) {
      throw std::out_of_range("");
    }
    return wall_colliders_[wall_index].shape.get();
  }

  engine::BoundingBox GetPillarsBoundingBox(const glm::mat4& transform) const {
    return pillars_.boundingBox(transform);
  }

  btCollisionShape* GetPillarCollisionShape() {
    return pillars_collider_.shape.get();
  }

 private:
  engine::MeshRenderer pillars_;
  engine::MeshRenderer walls_[4];
  engine::ShaderProgram prog_;
  engine::ShaderProgram shadow_prog_;

  struct PhysicsCollider {
    std::vector<int> indices;
    std::unique_ptr<btTriangleIndexVertexArray> triangles;
    std::unique_ptr<btCollisionShape> shape;
  } pillars_collider_, wall_colliders_[4];

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uShadowCP_, uModelMatrix_;
  gl::LazyUniform<glm::mat4> uSProjectionMatrix_, uSCameraMatrix_, uSModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
};

static WallRenderer& GetWallRenderer(engine::Scene* scene) {
  static WallRenderer renderer{scene};
  return renderer;
}

Wall::Wall(GameObject *parent, const engine::Transform& initial_transform)
    : GameObject(parent, initial_transform) {
  pillars_bb_ = GetWallRenderer(scene_).GetPillarsBoundingBox(transform().matrix());
  for (int i = 0; i < 4; ++i) {
    walls_bb_[i] = GetWallRenderer(scene_).GetWallBoundingBox(transform().matrix(), i);
  }
  for (int i = 0; i < 4; ++i) {
    wall_parts_up_[i] = (rand() % 10) != 0;
    if (wall_parts_up_[i]) {
      wall_part_colliders_[i] = AddComponent<engine::BulletRigidBody>(0.0f, GetWallRenderer(scene_).GetWallCollisionShape(i));
    }
  }

  AddComponent<engine::BulletRigidBody>(0.0f, GetWallRenderer(scene_).GetPillarCollisionShape());
}

engine::BoundingBox Wall::GetBoundingBox() const {
  return pillars_bb_;
}

double Wall::GetLength() const {
  return pillars_bb_.extent().x;
}

void Wall::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  for (int i = 0; i < 4; ++i) {
    if (glm::length(exp_position - walls_bb_[i].center()) < exp_radius) {
      wall_parts_up_[i] = false;
      RemoveComponent(wall_part_colliders_[i]);
    }
  }
}

void Wall::Render() {
  const auto& cam = *scene_->camera();
  auto& shadow_cam = *dynamic_cast<engine::Shadow*>(scene_->shadow_camera());
  gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
  shadow_cam.shadow_texture().compareMode(gl::kCompareRefToTexture);

  GetWallRenderer(scene_).Render(
      cam.projectionMatrix(),
      cam.cameraMatrix(),
      shadow_cam.projectionMatrix() * shadow_cam.cameraMatrix(),
      transform().matrix(),
      glm::inverse(glm::mat3(transform().matrix())),
      wall_parts_up_
  );

  gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
  shadow_cam.shadow_texture().compareMode(gl::kNone);
  gl::Unbind(shadow_cam.shadow_texture());
}

void Wall::ShadowRender() {
  const auto& shadow_cam = *scene_->shadow_camera();
  GetWallRenderer(scene_).ShadowRender(
      shadow_cam.projectionMatrix(),
      shadow_cam.cameraMatrix(),
      transform().matrix(),
      wall_parts_up_
  );
}
