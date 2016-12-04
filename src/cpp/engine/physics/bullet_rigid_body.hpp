// Copyright (c) Tamas Csala

#ifndef BULLET_RIGID_BODY_HPP_
#define BULLET_RIGID_BODY_HPP_

#include <btBulletDynamicsCommon.h>

#include "engine/game_object.hpp"

namespace engine {

class BulletRigidBody : public engine::GameObject, public btMotionState {
 public:
  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape);

  BulletRigidBody(GameObject* parent, float mass,
                  btCollisionShape* shape);

  BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                  const glm::vec3& pos);

  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape,
                  const glm::vec3& pos);

  BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                  const glm::vec3& pos, const glm::fquat& rot);

  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape,
                  const glm::vec3& pos, const glm::fquat& rot);

  virtual ~BulletRigidBody();

  btRigidBody* bt_rigid_body() { return bt_rigid_body_.get(); }
  const btRigidBody* bt_rigid_body() const { return bt_rigid_body_.get(); }

  struct Restrains {
    unsigned int x_pos_lock : 1;
    unsigned int y_pos_lock : 1;
    unsigned int z_pos_lock : 1;
    unsigned int manual_rot : 1;
    unsigned int x_rot_lock : 1;
    unsigned int y_rot_lock : 1;
    unsigned int z_rot_lock : 1;

    Restrains();
  };

  Restrains restrains() const { return restrains_; }
  void set_restrains(Restrains value);

 private:
  std::unique_ptr<btCollisionShape> shape_;
  std::unique_ptr<btRigidBody> bt_rigid_body_;
  bool up_to_date_;
  btTransform new_transform_;
  Restrains restrains_;

  void Init(float mass, btCollisionShape* shape);

  // GameObject virtual functions
  virtual void Update() override;

  // btMotionState virtual functions
  virtual void getWorldTransform(btTransform &t) const override;
  virtual void setWorldTransform(const btTransform &t) override;

};

}

#endif
