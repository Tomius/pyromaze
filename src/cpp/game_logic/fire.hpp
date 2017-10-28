// Copyright (c) Tamas Csala

#ifndef FIRE_HPP_
#define FIRE_HPP_

#include <Silice3D/common/oglwrap.hpp>
#include <oglwrap/shapes/cube_shape.h>

#include <Silice3D/core/game_object.hpp>
#include <Silice3D/shaders/shader_manager.hpp>

struct Particle {
  glm::vec3 pos, speed, accel;
  float born_at = -1, lifespan = -1;
  float scale = 0;

  Particle() = default; // dead particle
  Particle(glm::vec3 startpos, float current_time);
  bool IsAlive(float current_time);
  void Update(float dt);
};

typedef Particle (*ParticleGen)(glm::vec3 startpos, float current_time);

class ParticleSystem : public Silice3D::GameObject {
 public:
  ParticleSystem(GameObject* parent, ParticleGen generator,
                 int max_particles_at_once, int max_particle_per_sec,
                 int max_partice_count = -1);

 protected:
  gl::CubeShape cube_;

  Silice3D::ShaderProgram prog_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<float> uLifeTime_;

  ParticleGen generator_;
  float newParticlesToSpawn_ = 0.0;
  std::vector<Particle> particles_;
  int particles_generated_ = 0;
  int max_particles_at_once_, max_particle_per_sec_, max_particle_count_;

  virtual void Update() override;
  virtual void Render() override;
};

class Fire : public ParticleSystem {
public:
  Fire(GameObject* parent);

private:
  virtual void Update() override;
};

class Explosion : public ParticleSystem {
public:
  Explosion(GameObject* parent);

private:
  Silice3D::PointLightSource* light_source = nullptr;
  float born_at_;

  virtual void Update() override;
};

#endif
