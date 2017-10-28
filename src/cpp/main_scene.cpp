// Copyright (c) Tamas Csala

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
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

  AddComponent<Skybox>("src/resource/skybox.png");

  // must be the first object after skybox
  AddComponent<Silice3D::MeshObjectBatchRenderer>();

  cameras_ = AddComponent<Silice3D::GameObject>();

  player_camera_ = cameras_->AddComponent<Silice3D::BulletFreeFlyCamera>(
      M_PI/3, 1, Settings::kLabyrinthDiameter*kWallLength, glm::vec3(16, 3, 8), glm::vec3(10, 3, 8), 16, 10);
  set_camera(player_camera_);

  Player* player = player_camera_->AddComponent<Player>();

  // Shadows must be added after the cameras (update order!)
  const glm::vec3 lightPos = glm::normalize(glm::vec3{1.0});
  const glm::vec3 lightColor {0.50f};
  const size_t shadow_map_size = 1 << 11;
  const size_t shadow_cascades_count = 4;
  constexpr bool multiLight = false;
  if (multiLight) {
    Silice3D::DirectionalLightSource* light_source = AddComponent<Silice3D::DirectionalLightSource>(
      glm::vec3{0, 1.0f, 0}, shadow_map_size, shadow_cascades_count);
    light_source->transform().SetPos(lightPos);

    Silice3D::DirectionalLightSource* light_source2 = AddComponent<Silice3D::DirectionalLightSource>(
        glm::vec3{0, 0, 1.0f}, shadow_map_size, shadow_cascades_count);
    light_source2->transform().SetPos(glm::vec3{-lightPos.x, lightPos.y, lightPos.z});

    Silice3D::DirectionalLightSource* light_source3 = AddComponent<Silice3D::DirectionalLightSource>(
        glm::vec3{1.0f, 0.0f, 0}, shadow_map_size, shadow_cascades_count);
    light_source3->transform().SetPos(glm::vec3{-lightPos.x, lightPos.y, -lightPos.z});
  } else {
    Silice3D::DirectionalLightSource* light_source = AddComponent<Silice3D::DirectionalLightSource>(
      lightColor, shadow_map_size, shadow_cascades_count);
    light_source->transform().SetPos(lightPos);
  }

  CreateLabyrinth(player);

  AddComponent<Silice3D::FpsDisplay>();
}

void MainScene::CreateLabyrinth(Player* player) {
  auto envir = AddComponent<GameObject>();

  envir->AddComponent<Ground>();

  for (int x = -Settings::kLabyrinthRadius; x <= Settings::kLabyrinthRadius; ++x) {
    for (int z = -Settings::kLabyrinthRadius; z <= Settings::kLabyrinthRadius; ++z) {
      Silice3D::Transform wall_transform;
      wall_transform.SetLocalPos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<Wall>(wall_transform);

      if ((abs(x) > 1 || abs(z) > 1) && x != Settings::kLabyrinthRadius
          && z != Settings::kLabyrinthRadius && rand()%2 == 0) {
        Silice3D::Transform robot_transform;
        robot_transform.SetLocalPos({x * kWallLength + kWallLength/2.0, 3,
                                       z * kWallLength + kWallLength/2.0});
        envir->AddComponent<Robot>(robot_transform, player);
      }
    }
  }

  constexpr int kBorderRadius = Settings::kLabyrinthRadius+1;
  for (int z = -kBorderRadius; z <= kBorderRadius; z += 2*kBorderRadius) {
    for (int x = -kBorderRadius; x <= kBorderRadius; ++x) {
      Silice3D::Transform wall_transform;
      wall_transform.SetLocalPos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall1.obj", wall_transform);
    }
  }

  for (int x = -kBorderRadius; x <= kBorderRadius; x += 2*kBorderRadius) {
    for (int z = -kBorderRadius; z <= kBorderRadius; ++z) {
      Silice3D::Transform wall_transform;
      wall_transform.SetLocalPos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall2.obj", wall_transform);
    }
  }
}

void MainScene::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
    auto eng = engine();
    eng->LoadScene(std::unique_ptr<Silice3D::Scene>{new MainScene{eng, eng->window()}});
  } else if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
    static bool frozen = false;
    frozen = !frozen;

    if (frozen) {
      // freeze the scene now
      game_time().Stop();
      Silice3D::ICamera* free_fly_cam = cameras_->AddComponent<Silice3D::FreeFlyCamera>(
        M_PI/3, 1, Settings::kLabyrinthDiameter*kWallLength,
        player_camera_->transform().GetPos(),
        player_camera_->transform().GetPos() + player_camera_->transform().GetForward(),
        16, 10);
      set_camera(free_fly_cam);
    } else {
      // unfreeze the scene now
      game_time().Start();
      cameras_->RemoveComponent(camera());
      set_camera(player_camera_);
    }
  }
}
