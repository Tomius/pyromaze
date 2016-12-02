// Copyright (c) Tamas Csala

#include "engine/camera/perspective_camera.hpp"
#include "engine/scene.hpp"

namespace engine {

class CameraTransform : public Transform {
 public:
  CameraTransform() : up_(vec3{0, 1, 0}) {}

  // We shouldn't inherit the parent's rotation, like how a normal Transform does
  virtual const quat rot() const override { return rot_; }
  virtual void set_rot(const quat& new_rot) override { rot_ = new_rot; }

  // We have custom up and right vectors
  virtual vec3 up() const override { return up_; }
  virtual void set_up(const vec3& new_up) override {
    up_ = glm::normalize(new_up);
  }
  virtual vec3 right() const override {
    return glm::normalize(glm::cross(forward(), up()));
  }

  virtual void set_right(const vec3& new_right) override {
    set_forward(glm::cross(up(), new_right));
  }

 private:
  vec3 up_;
};

PerspectiveCamera::PerspectiveCamera(GameObject* parent, float fovy, float z_near, float z_far)
    : ICamera(parent, CameraTransform{}), fovy_(fovy), z_near_(z_near)
    , z_far_(z_far), width_(0), height_(0) {
  assert(fovy_ < M_PI);
}

void PerspectiveCamera::ScreenResized(size_t width, size_t height) {
  width_ = width;
  height_ = height;
}

void PerspectiveCamera::UpdateCache() {
  UpdateCameraMatrix();
  UpdateProjectionMatrix();
  UpdateFrustum();
}

void PerspectiveCamera::UpdateCameraMatrix() {
  const Transform& t = transform();
  cam_mat_ = glm::lookAt(t.pos(), t.pos()+t.forward(), t.up());
}

void PerspectiveCamera::UpdateProjectionMatrix() {
  proj_mat_ = glm::perspectiveFov<float>(fovy_, width_, height_, z_near_, z_far_);
}

void PerspectiveCamera::UpdateFrustum() {
  glm::mat4 m = proj_mat_ * cam_mat_;

  // REMEMBER: m[i][j] is j-th row, i-th column (glm is column major)

  frustum_ = Frustum{{
    // left
   {m[0][3] + m[0][0],
    m[1][3] + m[1][0],
    m[2][3] + m[2][0],
    m[3][3] + m[3][0]},

    // right
   {m[0][3] - m[0][0],
    m[1][3] - m[1][0],
    m[2][3] - m[2][0],
    m[3][3] - m[3][0]},

    // top
   {m[0][3] - m[0][1],
    m[1][3] - m[1][1],
    m[2][3] - m[2][1],
    m[3][3] - m[3][1]},

    // bottom
   {m[0][3] + m[0][1],
    m[1][3] + m[1][1],
    m[2][3] + m[2][1],
    m[3][3] + m[3][1]},

    // near
   {m[0][2],
    m[1][2],
    m[2][2],
    m[3][2]},

    // far
   {m[0][3] - m[0][2],
    m[1][3] - m[1][2],
    m[2][3] - m[2][2],
    m[3][3] - m[3][2]}
  }}; // ctor normalizes the planes
}

}  // namespace engine
