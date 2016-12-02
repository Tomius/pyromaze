// Copyright (c) Tamas Csala

#ifndef BULLET_RIGID_BODY_HPP_
#define BULLET_RIGID_BODY_HPP_

#include <btBulletDynamicsCommon.h>

#include "engine/game_object.hpp"

namespace engine {

class BulletRigidBody : public engine::GameObject, public btMotionState {
 public:
  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape,
                  bool ignore_rotation = false);

  BulletRigidBody(GameObject* parent, float mass,
                  btCollisionShape* shape, bool ignore_rotation = false);

  BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                  const glm::vec3& pos, bool ignore_rotation = false);

  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape,
                  const glm::vec3& pos, bool ignore_rotation = false);

  BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                  const glm::vec3& pos, const glm::fquat& rot,
                  bool ignore_rotation = false);

  BulletRigidBody(GameObject* parent, float mass,
                  std::unique_ptr<btCollisionShape>&& shape,
                  const glm::vec3& pos, const glm::fquat& rot,
                  bool ignore_rotation = false);

  virtual ~BulletRigidBody();

  btRigidBody* bt_rigid_body() { return bt_rigid_body_.get(); }
  const btRigidBody* bt_rigid_body() const { return bt_rigid_body_.get(); }

 private:
  std::unique_ptr<btCollisionShape> shape_;
  std::unique_ptr<btRigidBody> bt_rigid_body_;
  bool ignore_rotation_, up_to_date_;
  btTransform new_transform_;

  void Init(float mass, btCollisionShape* shape);

  // GameObject virtual functions
  virtual void Update() override;

  // btMotionState virtual functions
  virtual void getWorldTransform(btTransform &t) const override;
  virtual void setWorldTransform(const btTransform &t) override;

};

}

#endif
