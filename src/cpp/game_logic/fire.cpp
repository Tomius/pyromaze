// Copyright (c) Tamas Csala

#include <Silice3D/common/math.hpp>
#include <Silice3D/core/scene.hpp>

#include "game_logic/fire.hpp"
#include "game_logic/explodable.hpp"

bool Particle::IsAlive(float current_time) {
  return born_at + lifespan > current_time;
}

void Particle::Update(float dt) {
  pos += speed * dt;
  speed += accel * dt;
}

ParticleSystem::ParticleSystem(GameObject* parent, ParticleGen generator,
                               int max_particles_at_once, int max_particle_per_sec,
                               int max_particle_count)
    : GameObject(parent)
    , cube_({gl::CubeShape::kPosition})
    , prog_{scene_->shader_manager()->get("fire.vert"),
            scene_->shader_manager()->get("fire.frag")}
    , uProjectionMatrix_(prog_, "uProjectionMatrix")
    , uCameraMatrix_(prog_, "uCameraMatrix")
    , uModelMatrix_{prog_, "uModelMatrix"}
    , uLifeTime_{prog_, "uLifeTime"}
    , generator_{generator}
    , max_particles_at_once_{max_particles_at_once}
    , max_particle_per_sec_{max_particle_per_sec}
    , max_particle_count_{max_particle_count} {
  gl::Use(prog_);
  prog_.validate();
  (prog_ | "aPosition").bindLocation(cube_.kPosition);

  particles_.resize(max_particles_at_once_);
  gl::Unuse(prog_);
}

void ParticleSystem::Update() {
  float current_time = scene_->game_time().GetCurrentTime();
  float dt = scene_->game_time().GetDeltaTime();
  newParticlesToSpawn_ += dt * max_particle_per_sec_;

  if (max_particle_count_ >= particles_generated_) {
    bool is_particle_still_alive = false;
    for (Particle& particle : particles_) {
      if (particle.IsAlive(current_time)) {
        is_particle_still_alive = true;
      }
    }
    if (!is_particle_still_alive) {
      parent()->RemoveComponent(this);
      return;
    }
  }

  for (Particle& particle : particles_) {
    if (particle.IsAlive(current_time)) {
      particle.Update(dt);
    } else if (newParticlesToSpawn_ >= 1 &&
               (max_particle_count_ < 0 ||
                particles_generated_ < max_particle_count_)) {
      particle = generator_(transform().GetPos(), current_time);
      --newParticlesToSpawn_;
      particles_generated_++;
    }
  }
}

void ParticleSystem::Render() {
  gl::Use(prog_);
  prog_.Update();

  auto cam = scene_->camera();
  uCameraMatrix_ = cam->GetCameraMatrix();
  uProjectionMatrix_ = cam->GetProjectionMatrix();

  float current_time = scene_->game_time().GetCurrentTime();

  gl::TemporaryEnable blend{gl::kBlend};
  gl::BlendFunc(gl::kSrcAlpha, gl::kOneMinusSrcAlpha);

  for (Particle& particle : particles_) {
    if (particle.IsAlive(current_time)) {
      uLifeTime_ = current_time - particle.born_at;
      glm::vec3 scale{particle.scale};
      uModelMatrix_.set(glm::translate(particle.pos) * glm::scale(scale));
      cube_.render();
    }
  }
  gl::Unuse(prog_);
}

Particle FireParticle(glm::vec3 startpos, float current_time) {
  Particle p;
  p.born_at = current_time;
  p.pos = startpos;

  // Make the particles converge at (0, 4, 0)
  p.accel = 0.5f*normalize(startpos + glm::vec3{0, 4, 0} - p.pos + 0.2f*Silice3D::Math::RandomDir());
  p.speed = 0.2f*Silice3D::Math::RandomDir() + 4.0f*p.accel;

  // 1 - 3 sec lifespan
  p.lifespan = (rand()%20) / 10.0 + 1;

  p.scale = 0.025 + 0.025*Silice3D::Math::Rand01();
  return p;
}

Particle ExplosionParticle(glm::vec3 startpos, float current_time) {
  Particle p;
  p.born_at = current_time;
  p.pos = startpos + Silice3D::Math::RandomDir();
  p.accel = (5.0f + 5.0f*Silice3D::Math::Rand01())*normalize(Silice3D::Math::RandomDir());
  // p.accel.y = std::max(p.accel.y, 0.0f);
  p.speed = 2.0f*p.accel;

  // 0.5 - 1 sec lifespan
  p.lifespan = (rand()%5) / 10.0 + 0.5;

  p.scale = 0.1 + 0.1*Silice3D::Math::Rand01();
  return p;
}


Fire::Fire(GameObject* parent)
    : ParticleSystem(parent, FireParticle, 1000, 200) {
  glm::vec3 color = glm::vec3{2.0f};
  glm::vec3 attenuation = glm::vec3{1, 0.1, 0.1};
  AddComponent<Silice3D::PointLightSource>(color, attenuation);
}

void Fire::Update() {
  ParticleSystem::Update();
}

Explosion::Explosion(GameObject* parent)
    : ParticleSystem(parent, ExplosionParticle, 2800, 0, 3000) {
  glm::vec3 color = glm::vec3{1.0f};
  glm::vec3 attenuation = glm::vec3{1, 0.1, 0.1};
  light_source = AddComponent<Silice3D::PointLightSource>(color, attenuation);
  born_at_ = scene_->game_time().GetCurrentTime();
}

void Explosion::Update() {
  float current_time = scene_->game_time().GetCurrentTime();
  for (Particle& particle : particles_) {
    if (!particle.IsAlive(current_time) && rand()%8 == 0 &&
        particles_generated_ < max_particle_count_) {
      particle = generator_(transform().GetPos(), current_time);
      particles_generated_++;
    }
  }
  float life_time = current_time - born_at_;
  if (life_time < 0.5) {
    scene_->EnumerateChildren(true, [&](Silice3D::GameObject* obj) {
      Explodable* explodable = dynamic_cast<Explodable*>(obj);
      if (explodable != nullptr) {
        explodable->ReactToExplosion(transform().GetPos(), 10);
      }
    });
  }
  float lightness = std::min(std::pow(100000, 1.0-sqrt(life_time)), 100.0);
  light_source->set_color(glm::vec3{lightness});
  ParticleSystem::Update();
}
