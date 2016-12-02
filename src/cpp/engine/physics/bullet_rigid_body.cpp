// Copyright (c) Tamas Csala

#include "./bullet_rigid_body.hpp"

#include "engine/scene.hpp"
#include "engine/common/make_unique.hpp"

namespace engine {


BulletRigidBody::BulletRigidBody(GameObject* parent, float mass,
                                 std::unique_ptr<btCollisionShape>&& shape,
                                 bool ignore_rotation /*= false*/)
    : GameObject(parent), shape_(std::move(shape))
    , ignore_rotation_(ignore_rotation), up_to_date_(true) {
  Init(mass, shape_.get());
}

BulletRigidBody::BulletRigidBody(GameObject* parent, float mass,
                                 btCollisionShape* shape,
                                 bool ignore_rotation /*= false*/)
    : GameObject(parent), ignore_rotation_(ignore_rotation), up_to_date_(true) {
  Init(mass, shape);
}

BulletRigidBody::BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                                 const glm::vec3& pos, bool ignore_rotation /*= false*/)
    : GameObject(parent), ignore_rotation_(ignore_rotation), up_to_date_(true) {
  transform().set_pos(pos);
  Init(mass, shape);
}

BulletRigidBody::BulletRigidBody(GameObject* parent, float mass,
                                 std::unique_ptr<btCollisionShape>&& shape,
                                 const glm::vec3& pos, bool ignore_rotation /*= false*/)
    : GameObject(parent), shape_(std::move(shape))
    , ignore_rotation_(ignore_rotation), up_to_date_(true) {
  transform().set_pos(pos);
  Init(mass, shape_.get());
}

BulletRigidBody::BulletRigidBody(GameObject* parent, float mass, btCollisionShape* shape,
                                 const glm::vec3& pos, const glm::fquat& rot,
                                 bool ignore_rotation /*= false*/)
    : GameObject(parent), ignore_rotation_(ignore_rotation), up_to_date_(true) {
  transform().set_pos(pos);
  transform().set_rot(rot);
  Init(mass, shape);
}

BulletRigidBody::BulletRigidBody(GameObject* parent, float mass,
                                 std::unique_ptr<btCollisionShape>&& shape,
                                 const glm::vec3& pos, const glm::fquat& rot,
                                 bool ignore_rotation /*= false*/)
    : GameObject(parent), shape_(std::move(shape))
    , ignore_rotation_(ignore_rotation), up_to_date_(true) {
  transform().set_pos(pos);
  transform().set_rot(rot);
  Init(mass, shape.get());
}

BulletRigidBody::~BulletRigidBody() {
  scene_->bt_world()->removeCollisionObject(bt_rigid_body_.get());
}

void BulletRigidBody::Init(float mass, btCollisionShape* shape) {
  btVector3 inertia(0, 0, 0);
  if (mass != 0.0f) {
    shape->calculateLocalInertia(mass, inertia);
  }
  btRigidBody::btRigidBodyConstructionInfo info{mass, this, shape, inertia};
  bt_rigid_body_ = engine::make_unique<btRigidBody>(info);
  bt_rigid_body_->setUserPointer(parent_);
  if (mass == 0.0f) { bt_rigid_body_->setRestitution(1.0f); }
  scene_->bt_world()->addRigidBody(bt_rigid_body_.get());
}

void BulletRigidBody::getWorldTransform(btTransform &t) const {
  if (up_to_date_) {
    const glm::vec3& pos = transform().pos();
    t.setOrigin(btVector3{pos.x, pos.y, pos.z});
    if (!ignore_rotation_) {
      const glm::fquat& rot = transform().rot();
      t.setRotation(btQuaternion{rot.x, rot.y, rot.z, rot.w});
    }
  } else {
    t = new_transform_;
  }
}

void BulletRigidBody::setWorldTransform(const btTransform &t) {
  new_transform_ = t;
  up_to_date_ = false;
}

void BulletRigidBody::Update() {
  if (!up_to_date_) {
    const btVector3& o = new_transform_.getOrigin();
    parent_->transform().set_pos(glm::vec3(o.x(), o.y(), o.z()));
    if (!ignore_rotation_) {
      const btQuaternion& r = new_transform_.getRotation();
      parent_->transform().set_rot(glm::quat(r.getW(), r.getX(),
                                             r.getY(), r.getZ()));
    }
    up_to_date_ = true;
  }
}

}

