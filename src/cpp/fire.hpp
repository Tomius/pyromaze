#ifndef FIRE_HPP_
#define FIRE_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

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
Particle FireParticle(glm::vec3 startpos, float current_time);
Particle ExplosionParticle(glm::vec3 startpos, float current_time);

class ParticleSystem : public engine::GameObject {
 public:
  ParticleSystem(GameObject* parent, ParticleGen generator,
                 int max_particles_at_once, int max_particle_per_sec,
                 int max_partice_count = -1);

 protected:
  gl::CubeShape cube_;

  engine::ShaderProgram prog_;
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

struct Fire : public ParticleSystem {
  Fire(GameObject* parent) : ParticleSystem(parent, FireParticle, 1000, 200) {}
};

struct Explosion : public ParticleSystem {
  Explosion(GameObject* parent);

  virtual void Update() override;
};

#endif
