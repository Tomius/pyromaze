#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"
#include "game_logic/fire.hpp"
#include "engine/scene.hpp"
#include "settings.hpp"

Robot::Robot(engine::GameObject* parent, const engine::Transform& initial_transform,
             Player* player)
    : MeshObject(parent, "robot.obj", initial_transform), player_(player) {
  rbody_ = AddComponent<engine::BulletRigidBody>(1.0f, engine::make_unique<btSphereShape>(1.0), engine::kColDynamic);
  engine::BulletRigidBody::Restrains restrains;
  restrains.y_pos_lock = 1;
  restrains.x_rot_lock = 1;
  restrains.y_rot_lock = 1;
  restrains.z_rot_lock = 1;
  rbody_->set_restrains(restrains);
  rbody_->bt_rigid_body()->setGravity(btVector3{0, 0, 0});
}

void Robot::Update() {
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

  Player* player = nullptr;
  if (Optimizations::kAIBugFix) {
    player = player_;
  } else {
    scene_->EnumerateChildren([&](engine::GameObject* obj) {
      Player* p = dynamic_cast<Player*>(obj);
      if (p != nullptr) {
        player = p;
      }
    });
  }

  if (player != nullptr) {
    glm::vec3 to_player = player->transform().pos() - transform().pos();
    if (length(to_player) > kDetectionRadius) {
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
