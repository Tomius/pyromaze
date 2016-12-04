// Copyright (c) Tamas Csala

#ifndef ENGINE_CAMERA_BULLET_FREE_FLY_CAMERA_HPP_
#define ENGINE_CAMERA_BULLET_FREE_FLY_CAMERA_HPP_

#include "engine/camera/free_fly_camera.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "engine/scene.hpp"

namespace engine {

class BulletFreeFlyCamera : public engine::FreeFlyCamera {
 public:
  BulletFreeFlyCamera(GameObject* parent, float fov, float z_near,
                      float z_far, const glm::vec3& pos,
                      const glm::vec3& target = glm::vec3(),
                      float speed_per_sec = 5.0f,
                      float mouse_sensitivity = 1.0f)
      : FreeFlyCamera(parent, fov, z_near, z_far, pos, target,
                      speed_per_sec, mouse_sensitivity) {
    float radius = 3.0f * z_near;
    btCollisionShape* shape = new btSphereShape(radius);
    auto rbody = AddComponent<BulletRigidBody>(
      1.0f, std::unique_ptr<btCollisionShape>{shape});
    BulletRigidBody::Restrains restrains;
    restrains.y_pos_lock = 1;
    restrains.manual_rot = 1;
    rbody->set_restrains(restrains);
    bt_rigid_body_ = rbody->bt_rigid_body();
    bt_rigid_body_->setGravity(btVector3{0, 0, 0});
    bt_rigid_body_->setActivationState(DISABLE_DEACTIVATION);
    bt_rigid_body_->setMassProps(0.1f, btVector3(0, 0, 0));
    bt_rigid_body_->setFriction(0.0f);
    bt_rigid_body_->setRestitution(0.0f);

    bt_rigid_body_->setCcdMotionThreshold(radius);
    bt_rigid_body_->setCcdSweptSphereRadius(radius/2.0f);
  }

 private:
  btRigidBody* bt_rigid_body_;

  virtual void Update() override {
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

    // Calculate the offset
    glm::vec3 offset;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      offset += transform().forward();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      offset -= transform().forward();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      offset += transform().right();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      offset -= transform().right();
    }
    offset.y = 0;
    if (length(offset) > Math::kEpsilon) {
      offset = normalize(offset);
    }
    offset *= speed_per_sec_;

    // Update the "position"
    bt_rigid_body_->setLinearVelocity(btVector3{offset.x, offset.y, offset.z});

    UpdateCache();
  }
};

}

#endif  // ENGINE_CAMERA_BULLET_FREE_FLY_CAMERA_HPP_
