#include <Silice3D/core/scene.hpp>

#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"
#include "game_logic/fire.hpp"
#include "settings.hpp"

Robot::Robot(Silice3D::GameObject* parent, const Silice3D::Transform& initial_transform,
             Player* player)
    : Silice3D::MeshObject(parent, "robot.obj", initial_transform), player_(player) {
  rbody_ = AddComponent<Silice3D::BulletRigidBody>(1.0f, Silice3D::make_unique<btSphereShape>(1.0),
                                                 initial_transform.pos(), Silice3D::kColDynamic);
  Silice3D::BulletRigidBody::Restrains restrains;
  restrains.y_pos_lock = 1;
  restrains.x_rot_lock = 1;
  restrains.y_rot_lock = 1;
  restrains.z_rot_lock = 1;
  rbody_->set_restrains(restrains);
  rbody_->bt_rigid_body()->setGravity(btVector3{0, 0, 0});
  if (Optimizations::kSleepRobots) {
    btScalar dtime = scene_->game_time().current_time() + 1e10f;
    rbody_->bt_rigid_body()->setDeactivationTime(dtime);
  }
}

void Robot::Update() {
  MeshObject::Update();

  constexpr bool kRobotExplodes = false;
  constexpr float kTimeToExplode = 2.0f;
  constexpr float kSpeed = 9.0f;
  constexpr float kDetectionRadius = 15.0f;

  if (kRobotExplodes && activation_time_ > 0 &&
      scene_->game_time().current_time() - activation_time_ > kTimeToExplode) {
    parent()->RemoveComponent(this);
    GameObject* explosion = parent()->AddComponent<Explosion>();
    explosion->transform().set_local_pos(transform().local_pos());
  }

  if (player_ != nullptr) {
    glm::vec3 to_player = player_->transform().pos() - transform().pos();
    if (length(to_player) > kDetectionRadius) {
      rbody_->bt_rigid_body()->setLinearVelocity({0, 0, 0});
      if (Optimizations::kSleepRobots) {
        btScalar dtime = scene_->game_time().current_time() + 1e10f;
        rbody_->bt_rigid_body()->setDeactivationTime(dtime);
      }
      return;
    }

    rbody_->bt_rigid_body()->activate();
    if (activation_time_ < 0) {
      activation_time_ = scene_->game_time().current_time();
    }
    glm::vec3 dir = to_player;
    dir.y = 0;
    if (length(dir) > Math::kEpsilon) {
      dir = normalize(dir);
    }
    glm::vec3 speed = kSpeed * dir;
    rbody_->bt_rigid_body()->setLinearVelocity(btVector3{speed.x, speed.y, speed.z});
  }
}

void Robot::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  glm::vec3 pos = transform().pos();
  pos.y = 0;
  if (length(pos - exp_position) < 1.2f*exp_radius) {
    parent_->RemoveComponent(this);
  }
}
