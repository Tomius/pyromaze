// Copyright (c) 2016, Tamas Csala

#include "engine/camera.hpp"
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

FreeFlyCamera::FreeFlyCamera(GameObject* parent, float fov, float z_near,
                             float z_far, const glm::vec3& pos,
                             const glm::vec3& target /*= glm::vec3()*/,
                             float speed_per_sec /*= 5.0f*/,
                             float mouse_sensitivity /*= 5.0f*/)
    : PerspectiveCamera(parent, fov, z_near, z_far)
    , first_call_(true)
    , speed_per_sec_(speed_per_sec)
    , mouse_sensitivity_(mouse_sensitivity)
    , cos_max_pitch_angle_(0.98f) {
  transform().set_pos(pos);
  transform().set_forward(target - pos);
}

void FreeFlyCamera::Update() {
  glm::dvec2 cursor_pos;
  GLFWwindow* window = scene_->window();
  glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
  static glm::dvec2 prev_cursor_pos;
  glm::dvec2 diff = cursor_pos - prev_cursor_pos;
  prev_cursor_pos = cursor_pos;

  // We get invalid diff values at the startup
  if (first_call_) {
    diff = glm::dvec2(0, 0);
    first_call_ = false;
  }

  const float dt = scene_->camera_time().dt();

  // Mouse movement - update the coordinate system
  if (diff.x || diff.y) {
    float dx(diff.x * mouse_sensitivity_ / 10000);
    float dy(-diff.y * mouse_sensitivity_ / 10000);

    // If we are looking up / down, we don't want to be able
    // to rotate to the other side
    float dot_up_fwd = glm::dot(transform().up(), transform().forward());
    if (dot_up_fwd > cos_max_pitch_angle_ && dy > 0) {
      dy = 0;
    }
    if (dot_up_fwd < -cos_max_pitch_angle_ && dy < 0) {
      dy = 0;
    }

    transform().set_forward(transform().forward() +
                             transform().right()*dx +
                             transform().up()*dy);
  }

  // Update the position
  float ds = dt * speed_per_sec_;
  glm::vec3 local_pos = transform().local_pos();
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    local_pos += transform().forward() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    local_pos -= transform().forward() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    local_pos += transform().right() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    local_pos -= transform().right() * ds;
  }
  transform().set_local_pos(local_pos);

  PerspectiveCamera::UpdateCache();
}

ThirdPersonalCamera::ThirdPersonalCamera(GameObject* parent,
                                         float fov,
                                         float z_near,
                                         float z_far,
                                         const glm::vec3& position,
                                         float mouse_sensitivity /*= 1.0*/,
                                         float mouse_scroll_sensitivity /*= 1.0*/,
                                         float min_dist_mod /*= 0.25*/,
                                         float max_dist_mod /*= 4.00*/,
                                         float base_distance /*= 0.0*/,
                                         float dist_offset /*= 0.0*/)
    : PerspectiveCamera(parent, fov, z_near, z_far)
    , target_(parent->transform())
    , first_call_(true)
    , initial_distance_(glm::length(target_.pos() - position) - dist_offset)
    , base_distance_(base_distance == 0.0 ? initial_distance_ : base_distance)
    , cos_max_pitch_angle_(0.999)
    , mouse_sensitivity_(mouse_sensitivity)
    , mouse_scroll_sensitivity_(mouse_scroll_sensitivity)
    , min_dist_mod_(min_dist_mod)
    , max_dist_mod_(max_dist_mod)
    , dist_offset_(dist_offset)
    , curr_dist_mod_(initial_distance_ / base_distance_)
    , dest_dist_mod_(curr_dist_mod_){
  transform().set_pos(position);
  transform().set_forward(target_.pos() - position);
}

void ThirdPersonalCamera::Update() {
  static glm::dvec2 prev_cursor_pos;
  glm::dvec2 cursor_pos;
  GLFWwindow* window = scene_->window();
  glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
  glm::dvec2 diff = cursor_pos - prev_cursor_pos;
  prev_cursor_pos = cursor_pos;

  // We get invalid diff values at the startup
  if (first_call_ && (diff.x != 0 || diff.y != 0)) {
    diff = glm::dvec2(0, 0);
    first_call_ = false;
  }

  const float dt = scene_->camera_time().dt();

  // Mouse movement - update the coordinate system
  if (diff.x || diff.y) {
    float mouse_sensitivity = mouse_sensitivity_ * curr_dist_mod_ / 1000;
    float dx(diff.x * mouse_sensitivity);
    float dy(-diff.y * mouse_sensitivity);

    // If we are looking up / down, we don't want to be able
    // to rotate to the other side
    float dot_up_fwd = glm::dot(transform().up(), transform().forward());
    if (dot_up_fwd > cos_max_pitch_angle_ && dy > 0) {
      dy = 0;
    }
    if (dot_up_fwd < -cos_max_pitch_angle_ && dy < 0) {
      dy = 0;
    }

    transform().set_forward(transform().forward() +
                             transform().right()*dx +
                             transform().up()*dy);
  }

  float dist_diff_mod = dest_dist_mod_ - curr_dist_mod_;
  if (fabs(dist_diff_mod) > dt * mouse_scroll_sensitivity_) {
    curr_dist_mod_ *= dist_diff_mod > 0 ?
      (1 + dt * mouse_scroll_sensitivity_) :
      (1 - dt * mouse_scroll_sensitivity_);
  }

  // Update the position
  glm::vec3 tpos(target_.pos()), fwd(transform().forward());
  fwd = transform().forward();
  float dist = curr_dist_mod_*base_distance_ + dist_offset_;
  glm::vec3 pos = tpos - fwd*dist;
  transform().set_pos(pos);

  PerspectiveCamera::UpdateCache();
}

void ThirdPersonalCamera::MouseScrolled(double, double yoffset) {
  dest_dist_mod_ *= 1 + (-yoffset) * 0.1 * mouse_scroll_sensitivity_;
  if (dest_dist_mod_ < min_dist_mod_) {
    dest_dist_mod_ = min_dist_mod_;
  } else if (dest_dist_mod_ > max_dist_mod_) {
    dest_dist_mod_ = max_dist_mod_;
  }
}

}  // namespace engine
