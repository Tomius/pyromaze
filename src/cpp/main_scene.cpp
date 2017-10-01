#include "./main_scene.hpp"
#include "./settings.hpp"

#include "environment/ground.hpp"
#include "environment/wall.hpp"
#include "environment/skybox.hpp"
#include "environment/border_wall.hpp"

#include "game_logic/fire.hpp"
#include "game_logic/dynamite.hpp"
#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"

#include <Silice3D/core/game_engine.hpp>
#include <Silice3D/common/make_unique.hpp>
#include <Silice3D/camera/bullet_free_fly_camera.hpp>
#include <Silice3D/mesh/mesh_object_batch_renderer.hpp>
#include <Silice3D/debug/fps_display.hpp>
#include <Silice3D/debug/debug_shape.hpp>
#include <Silice3D/debug/debug_texture.hpp>

constexpr float kWallLength = 20;

MainScene::MainScene(Silice3D::GameEngine* engine, GLFWwindow* window)
    : Scene(engine, window) {
  if (!Settings::kDetermininistic) {
    srand(time(nullptr));
  }
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  { // Bullet initilization
    bt_collision_config_ = Silice3D::make_unique<btDefaultCollisionConfiguration>();
    bt_dispatcher_ = Silice3D::make_unique<btCollisionDispatcher>(bt_collision_config_.get());
    bt_broadphase_ = Silice3D::make_unique<btDbvtBroadphase>();
    bt_solver_ = Silice3D::make_unique<btSequentialImpulseConstraintSolver>();
    bt_world_ = Silice3D::make_unique<btDiscreteDynamicsWorld>(
        bt_dispatcher_.get(), bt_broadphase_.get(),
        bt_solver_.get(), bt_collision_config_.get());
    bt_world_->setGravity(btVector3(0, -9.81, 0));
  }

  shader_manager()->get("lighting.frag")->set_update_func([this](const gl::Program& prog) {
    // gl::Use(prog);
    int dir_light_count = 0, pos_light_count = 0;
    for (const auto& pair : light_sources_) {
      const LightSource& light = pair.second;
      if (light.type == LightSource::Type::kPoint) {
        std::string uniform_name = "uPointLights[";
        uniform_name += std::to_string(pos_light_count++);
        uniform_name += "]";
        gl::Uniform<glm::vec3>(prog, uniform_name + ".position") = light.position;
        gl::Uniform<glm::vec3>(prog, uniform_name + ".color") = light.color;
      } else if (light.type == LightSource::Type::kDirectional) {
        std::string uniform_name = "uDirectionalLights[";
        uniform_name += std::to_string(dir_light_count++);
        uniform_name += "]";
        gl::Uniform<glm::vec3>(prog, uniform_name + ".direction") = light.position;
        gl::Uniform<glm::vec3>(prog, uniform_name + ".color") = light.color;
      }
    }
    gl::Uniform<int>(prog, "uDirectionalLightCount") = dir_light_count;
    gl::Uniform<int>(prog, "uPointLightCount") = pos_light_count;
  });

  const glm::vec3 kLightPos = glm::normalize(glm::vec3{1.0});
  AddLightSource({LightSource::Type::kDirectional, kLightPos, glm::vec3{0.40f}});

  shadow_ = AddComponent<Silice3D::Shadow>(kLightPos, 4096);
  set_shadow(shadow_);

  AddComponent<Skybox>("src/resource/skybox.png");

  // must be the first object after skybox
  AddComponent<Silice3D::MeshObjectBatchRenderer>();

  auto cam = AddComponent<Silice3D::/*Bullet*/FreeFlyCamera>(
      M_PI/3, 1, Settings::kLabyrinthDiameter*kWallLength, glm::vec3(16, 3, 8), glm::vec3(10, 3, 8), 16, 10);
  set_camera(cam);

  // Silice3D::Transform playerTransform{&cam->transform()};
  Player* player = AddComponent<Player>(Silice3D::Transform{}/*playerTransform*/);

  CreateLabyrinth(player);

  AddComponent<Silice3D::FpsDisplay>();
}

void MainScene::CreateLabyrinth(Player* player) {
  auto envir = AddComponent<GameObject>();

  envir->AddComponent<Ground>();

  for (int x = -Settings::kLabyrinthRadius; x <= Settings::kLabyrinthRadius; ++x) {
    for (int z = -Settings::kLabyrinthRadius; z <= Settings::kLabyrinthRadius; ++z) {
      Silice3D::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<Wall>(wall_transform);

      if ((abs(x) > 1 || abs(z) > 1) && x != Settings::kLabyrinthRadius
          && z != Settings::kLabyrinthRadius && rand()%2 == 0) {
        Silice3D::Transform robot_transform;
        robot_transform.set_local_pos({x * kWallLength + kWallLength/2.0, 3,
                                       z * kWallLength + kWallLength/2.0});
        envir->AddComponent<Robot>(robot_transform, player);
      }
    }
  }

  constexpr int kBorderRadius = Settings::kLabyrinthRadius+1;
  for (int z = -kBorderRadius; z <= kBorderRadius; z += 2*kBorderRadius) {
    for (int x = -kBorderRadius; x <= kBorderRadius; ++x) {
      Silice3D::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall1.obj", wall_transform);
    }
  }

  for (int x = -kBorderRadius; x <= kBorderRadius; x += 2*kBorderRadius) {
    for (int z = -kBorderRadius; z <= kBorderRadius; ++z) {
      Silice3D::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall2.obj", wall_transform);
    }
  }
}

void MainScene::RenderAll() {
  shadow_->FillShadowMap(this);

#if 0
  auto& shadow_tex = shadow_->shadow_texture();
  auto tex_bind = gl::MakeTemporaryBind(shadow_tex);
  shadow_tex.compareMode(gl::kNone);
  Silice3D::DebugTexture(shader_manager()).Render(shadow_tex);
  shadow_tex.compareMode(gl::kCompareRefToTexture);
#else
  gl::BindToTexUnit(shadow_->shadow_texture(), Silice3D::kShadowTextureSlot);
  Scene::RenderAll();
  gl::UnbindFromTexUnit(shadow_->shadow_texture(), Silice3D::kShadowTextureSlot);
#endif
}

void MainScene::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
    auto eng = scene_->engine();
    eng->LoadScene(std::unique_ptr<Silice3D::Scene>{new MainScene{eng, eng->window()}});
  }
}
