#include "./main_scene.hpp"
#include "./ground.hpp"
#include "./wall.hpp"
#include "./skybox.hpp"
#include "./fire.hpp"
#include "./dynamite.hpp"
#include "./castle.hpp"
#include "./robot.hpp"
#include "./player.hpp"

#include "engine/common/make_unique.hpp"
#include "engine/camera/bullet_free_fly_camera.hpp"

#include "debug/debug_shape.hpp"
#include "debug/debug_texture.hpp"

constexpr int kWallLength = 20;
constexpr int kLabyrinthRadius = 6, kLabyrinthDiameter = 2*kLabyrinthRadius + 1;

MainScene::MainScene(engine::GameEngine* engine, GLFWwindow* window)
    : Scene(engine, window) {
  srand(time(nullptr));
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  { // Bullet initilization
    bt_collision_config_ = engine::make_unique<btDefaultCollisionConfiguration>();
    bt_dispatcher_ = engine::make_unique<btCollisionDispatcher>(bt_collision_config_.get());
    bt_broadphase_ = engine::make_unique<btDbvtBroadphase>();
    bt_solver_ = engine::make_unique<btSequentialImpulseConstraintSolver>();
    bt_world_ = engine::make_unique<btDiscreteDynamicsWorld>(
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
  AddLightSource({LightSource::Type::kDirectional, kLightPos, glm::vec3{0.1f}});

  shadow_ = AddComponent<engine::Shadow>(kLightPos, glm::vec4{0, 0, 0, kLabyrinthDiameter}, 4096);
  set_shadow_camera(shadow_);

  AddComponent<Skybox>("src/resource/skybox.png");

  CreateLabyrinth();

  auto cam = AddComponent<engine::BulletFreeFlyCamera>(
      M_PI/3, 1, 2000, glm::vec3(16, 3, 8), glm::vec3(10, 3, 8), 12, 10);
  set_camera(cam);

  engine::Transform playerTransform{&cam->transform()};
  AddComponent<Player>(playerTransform);
}

void MainScene::CreateLabyrinth() {
  //auto envir = AddComponent<GameObject>(); // TODO
  auto envir = this;

  // GameObject* castle = envir->AddComponent<Castle>();
  // castle->transform().set_scale({32, 32, 32});
  // castle->transform().set_local_pos({0, -12, 0});

  envir->AddComponent<Ground>();

  for (int x = -kLabyrinthRadius; x <= kLabyrinthRadius; ++x) {
    for (int z = -kLabyrinthRadius; z <= kLabyrinthRadius; ++z) {
      engine::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, 0, z * kWallLength});
      envir->AddComponent<Wall>(wall_transform);

      if (x != 0 && z != 0 && rand()%4 != 0) {
        engine::Transform robot_transform;
        robot_transform.set_local_pos({x * kWallLength + kWallLength/2.0, 3,
                                       z * kWallLength + kWallLength/2.0});
        AddComponent<Robot>(robot_transform);
      }
    }
  }

  constexpr int kBorderRadius = kLabyrinthRadius+1;
  for (int z = -kBorderRadius; z <= kBorderRadius; z += 2*kBorderRadius) {
    for (int x = -kBorderRadius; x <= kBorderRadius; ++x) {
      engine::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, 0, z * kWallLength});
      MeshObject* wall = envir->AddComponent<MeshObject>("wall/bigwall1.obj", wall_transform);
      wall->AddComponent<engine::BulletRigidBody>(0.0f, wall->GetCollisionShape());
    }
  }

  for (int x = -kBorderRadius; x <= kBorderRadius; x += 2*kBorderRadius) {
    for (int z = -kBorderRadius; z <= kBorderRadius; ++z) {
      engine::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, 0, z * kWallLength});
      MeshObject* wall = envir->AddComponent<MeshObject>("wall/bigwall2.obj", wall_transform);
      wall->AddComponent<engine::BulletRigidBody>(0.0f, wall->GetCollisionShape());
    }
  }
}

void MainScene::RenderAll() {
  shadow_->Begin();
  Scene::ShadowRenderAll();
  shadow_->End();

#if 0
  DebugTexture(shader_manager()).Render(shadow_->shadow_texture());
#else
  Scene::RenderAll();
#endif
}
