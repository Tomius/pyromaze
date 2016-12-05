#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"
#include "engine/scene.hpp"

Robot::Robot(engine::GameObject* parent, const engine::Transform& initial_transform)
    : MeshObject(parent, "robot.obj", initial_transform) {
  rbody_ = AddComponent<engine::BulletRigidBody>(1.0f, engine::make_unique<btSphereShape>(1.0), engine::kColDynamic);
  engine::BulletRigidBody::Restrains restrains;
  restrains.y_pos_lock = 1;
  restrains.x_rot_lock = 1;
  restrains.z_rot_lock = 1;
  rbody_->set_restrains(restrains);
  rbody_->bt_rigid_body()->setGravity(btVector3{0, 0, 0});
}

void Robot::Update() {
  scene_->EnumerateChildren([&](engine::GameObject* obj) {
    Player* player = dynamic_cast<Player*>(obj);
    if (player != nullptr) {
      glm::vec3 to_player = player->transform().pos() - transform().pos();
      if (length(to_player) > 12) {
        return;
      }
      rbody_->bt_rigid_body()->activate();
      glm::vec3 dir = normalize(to_player);
      glm::vec3 speed = 9.0f * dir;
      rbody_->bt_rigid_body()->setLinearVelocity(btVector3{speed.x, speed.y, speed.z});
    }
  });
}

void Robot::ReactToExplosion(const glm::vec3& exp_position, float exp_radius) {
  glm::vec3 pos = transform().pos();
  pos.y = 0;
  if (length(pos - exp_position) < 1.2f*exp_radius) {
    parent_->RemoveComponent(this);
  }
}
