#include "mesh_object.hpp"

#include "engine/scene.hpp"
#include "engine/shadow.hpp"

class MeshObjectRender {
public:
  MeshObjectRender (const std::string& mesh_path, engine::ShaderManager* shader_manager)
      : mesh_("src/resource/" + mesh_path, aiProcessPreset_TargetRealtime_Quality |
                                           aiProcess_FlipUVs |
                                           aiProcess_PreTransformVertices)
      , basic_prog_(shader_manager->get("mesh.vert"),
                    shader_manager->get("mesh.frag"))
      , shadow_recieve_prog_(shader_manager->get("mesh.vert"),
                             shader_manager->get("mesh_shadow.frag"))
      , shadow_cast_prog_(shader_manager->get("shadow.vert"),
                          shader_manager->get("shadow.frag"))

      , bp_uProjectionMatrix_(basic_prog_, "uProjectionMatrix")
      , bp_uCameraMatrix_(basic_prog_, "uCameraMatrix")
      , bp_uModelMatrix_(basic_prog_, "uModelMatrix")
      , bp_uNormalMatrix_(basic_prog_, "uNormalMatrix")

      , srp_uProjectionMatrix_(shadow_recieve_prog_, "uProjectionMatrix")
      , srp_uCameraMatrix_(shadow_recieve_prog_, "uCameraMatrix")
      , srp_uModelMatrix_(shadow_recieve_prog_, "uModelMatrix")
      , srp_uShadowCP_(shadow_recieve_prog_, "uShadowCP")
      , srp_uNormalMatrix_(shadow_recieve_prog_, "uNormalMatrix")

      , scp_uProjectionMatrix_(shadow_cast_prog_, "uProjectionMatrix")
      , scp_uCameraMatrix_(shadow_cast_prog_, "uCameraMatrix")
      , scp_uModelMatrix_(shadow_cast_prog_, "uModelMatrix") {
    gl::Use(basic_prog_);
    mesh_.setupPositions(basic_prog_ | "aPosition");
    mesh_.setupTexCoords(basic_prog_ | "aTexCoord");
    mesh_.setupNormals(basic_prog_ | "aNormal");
    mesh_.setupDiffuseTextures(1);
    gl::UniformSampler(basic_prog_, "uDiffuseTexture").set(1);
    basic_prog_.validate();

    gl::Use(shadow_recieve_prog_);
    gl::UniformSampler(shadow_recieve_prog_, "uShadowMap").set(0);
    gl::UniformSampler(shadow_recieve_prog_, "uDiffuseTexture").set(1);
    shadow_recieve_prog_.validate();
  }

  btCollisionShape* GetCollisionShape() {
    if (!bullet_data_.shape) {
      bullet_data_.triangles = engine::make_unique<btTriangleIndexVertexArray>();
      bullet_data_.indices = mesh_.btTriangles(bullet_data_.triangles.get());
      bullet_data_.shape = engine::make_unique<btBvhTriangleMeshShape>(bullet_data_.triangles.get(), true);
    }

    return bullet_data_.shape.get();
  }

  void Render(engine::Scene* scene,
              const engine::Transform& transform,
              bool recieve_shadows) {
    const auto& cam = *scene->camera();
    if (recieve_shadows) {
      auto& shadow_cam = *dynamic_cast<engine::Shadow*>(scene->shadow_camera());

      gl::Use(shadow_recieve_prog_);
      shadow_recieve_prog_.update();

      gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
      shadow_cam.shadow_texture().compareMode(gl::kCompareRefToTexture);

      srp_uProjectionMatrix_ = cam.projectionMatrix();
      srp_uCameraMatrix_ = cam.cameraMatrix();
      srp_uModelMatrix_ = transform.matrix();
      srp_uNormalMatrix_ = glm::inverse(glm::mat3(transform.matrix()));
      srp_uShadowCP_ = shadow_cam.projectionMatrix() * shadow_cam.cameraMatrix();

      mesh_.render();

      gl::BindToTexUnit(shadow_cam.shadow_texture(), 0);
      shadow_cam.shadow_texture().compareMode(gl::kNone);
      gl::Unbind(shadow_cam.shadow_texture());
    } else {
      gl::Use(basic_prog_);
      basic_prog_.update();

      bp_uProjectionMatrix_ = cam.projectionMatrix();
      bp_uCameraMatrix_ = cam.cameraMatrix();
      bp_uModelMatrix_ = transform.matrix();
      bp_uNormalMatrix_ = glm::inverse(glm::mat3(transform.matrix()));

      mesh_.render();
    }
  }

  void ShadowRender(engine::Scene* scene,
                    const engine::Transform& transform,
                    bool cast_shadows) {
    if (cast_shadows) {
      const auto& shadow_cam = *scene->shadow_camera();

      gl::Use(shadow_cast_prog_);
      shadow_cast_prog_.update();

      scp_uProjectionMatrix_ = shadow_cam.projectionMatrix();
      scp_uCameraMatrix_ = shadow_cam.cameraMatrix();
      scp_uModelMatrix_ = transform.matrix();

      mesh_.render();
    }
  }

  engine::BoundingBox GetBoundingBox(const glm::mat4& transform) const {
    return mesh_.boundingBox(transform);
  }

private:
  engine::MeshRenderer mesh_;

  engine::ShaderProgram basic_prog_;
  engine::ShaderProgram shadow_recieve_prog_;
  engine::ShaderProgram shadow_cast_prog_;

  // basic_prog uniforms
  gl::LazyUniform<glm::mat4> bp_uProjectionMatrix_, bp_uCameraMatrix_, bp_uModelMatrix_;
  gl::LazyUniform<glm::mat3> bp_uNormalMatrix_;

  // shadow_recieve_prog_ uniforms
  gl::LazyUniform<glm::mat4> srp_uProjectionMatrix_, srp_uCameraMatrix_, srp_uModelMatrix_, srp_uShadowCP_;
  gl::LazyUniform<glm::mat3> srp_uNormalMatrix_;

  // shadow_cast_prog_ uniforms
  gl::LazyUniform<glm::mat4> scp_uProjectionMatrix_, scp_uCameraMatrix_, scp_uModelMatrix_;

  struct BulletData {
    std::vector<int> indices;
    std::unique_ptr<btTriangleIndexVertexArray> triangles;
    std::unique_ptr<btCollisionShape> shape;
  } bullet_data_;
};

static MeshObjectRender& GetMeshRenderer(const std::string& str, engine::ShaderManager* shader_manager) {
  static std::map<std::string, std::unique_ptr<MeshObjectRender>> mesh_cache;
  auto iter = mesh_cache.find(str);
  if (iter == mesh_cache.end()) {
    return *(mesh_cache[str] = engine::make_unique<MeshObjectRender>(str, shader_manager));
  } else {
    return *iter->second;
  }
}

MeshObject::MeshObject(engine::GameObject* parent, const std::string& mesh_path,
                       const engine::Transform& initial_transform)
    : GameObject(parent, initial_transform)
    , renderer_{GetMeshRenderer(mesh_path, scene_->shader_manager())}
{ }

btCollisionShape* MeshObject::GetCollisionShape() {
  return renderer_.GetCollisionShape();
}

engine::BoundingBox MeshObject::GetBoundingBox() const {
  return renderer_.GetBoundingBox(transform().matrix());
}

void MeshObject::Render() {
  renderer_.Render(scene_, transform(), recieve_shadows_);
}

void MeshObject::ShadowRender() {
  renderer_.ShadowRender(scene_, transform(), cast_shadows_);
}
