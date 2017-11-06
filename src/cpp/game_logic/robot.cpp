// Copyright (c) Tamas Csala

#include <Silice3D/core/scene.hpp>

#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"
#include "game_logic/fire.hpp"
#include "settings.hpp"

Robot::Robot(Silice3D::GameObject* parent, const Silice3D::Transform& initial_transform,
             Player* player)
    : Silice3D::MeshObject(parent, "robot.obj", initial_transform), player_(player) {
  rbody_ = AddComponent<Silice3D::BulletRigidBody>(1.0f, Silice3D::make_unique<btSphereShape>(1.0),
                                                   initial_transform.GetPos(), Silice3D::kColDynamic);
  Silice3D::BulletRigidBody::Restrains restrains;
  restrains.y_pos_lock = 1;
  restrains.x_rot_lock = 1;
  restrains.y_rot_lock = 1;
  restrains.z_rot_lock = 1;
  rbody_->set_restrains(restrains);
  rbody_->bt_rigid_body()->setGravity(btVector3{0, 0, 0});
  rbody_->bt_rigid_body()->setActivationState(WANTS_DEACTIVATION);
}

void Robot::Update() {
  MeshObject::Update();

  constexpr bool kRobotExplodes = false;
  constexpr double kTimeToExplode = 2.0f;
  constexpr double kSpeed = 9.0f;
  constexpr double kDetectionRadius = 15.0f;

  if (kRobotExplodes && activation_time_ > 0 &&
      scene_->GetGameTime().GetCurrentTime() - activation_time_ > kTimeToExplode) {
    GetParent()->RemoveComponent(this);
    GameObject* explosion = GetParent()->AddComponent<Explosion>();
    explosion->GetTransform().SetLocalPos(GetTransform().GetLocalPos());
    return;
  }

  if (player_ != nullptr) {
    glm::dvec3 to_player = player_->GetTransform().GetPos() - GetTransform().GetPos();
    if (length(to_player) > kDetectionRadius) {
      rbody_->bt_rigid_body()->setLinearVelocity({0, 0, 0});
      rbody_->bt_rigid_body()->setActivationState(WANTS_DEACTIVATION);
      return;
    }

    rbody_->bt_rigid_body()->activate();
    if (activation_time_ < 0) {
      activation_time_ = scene_->GetGameTime().GetCurrentTime();
    }
    glm::dvec3 dir = to_player;
    dir.y = 0;
    if (length(dir) > Silice3D::Math::kEpsilon) {
      dir = normalize(dir);
    }
    glm::dvec3 speed = kSpeed * dir;
    rbody_->bt_rigid_body()->setLinearVelocity(btVector3{speed.x, speed.y, speed.z});
  }
}

void Robot::ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) {
  glm::dvec3 pos = GetTransform().GetPos();
  pos.y = 0;
  if (length(pos - exp_position) < 1.2f*exp_radius) {
    parent_->RemoveComponent(this);
  }
}
