#include "./main_scene.hpp"
#include "./fps_display.hpp"

#include "environment/ground.hpp"
#include "environment/wall.hpp"
#include "environment/skybox.hpp"
#include "environment/border_wall.hpp"

#include "game_logic/fire.hpp"
#include "game_logic/dynamite.hpp"
#include "game_logic/robot.hpp"
#include "game_logic/player.hpp"

#include "engine/common/make_unique.hpp"
#include "engine/camera/bullet_free_fly_camera.hpp"

#include "debug/debug_shape.hpp"
#include "debug/debug_texture.hpp"
#include "debug/debug_voxels.hpp"

constexpr int kWallLength = 20;
constexpr int kLabyrinthRadius = 5;

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
  AddLightSource({LightSource::Type::kDirectional, kLightPos, glm::vec3{0.05f}});

  shadow_ = AddComponent<engine::Shadow>(kLightPos, glm::vec4{0, 0, 0, (kLabyrinthRadius+1)*kWallLength}, 4096);
  set_shadow_camera(shadow_);

  vct_ = AddComponent<engine::VCT>();

  AddComponent<Skybox>("src/resource/skybox.png");

  CreateLabyrinth();

  auto cam = AddComponent<engine::BulletFreeFlyCamera>(
      M_PI/3, 1, 2000, glm::vec3(16, 3, 8), glm::vec3(10, 3, 8), 16, 10);
  set_camera(cam);

  engine::Transform playerTransform{&cam->transform()};
  AddComponent<Player>(playerTransform);

  AddComponent<FpsDisplay>();
}

void MainScene::CreateLabyrinth() {
  //auto envir = AddComponent<GameObject>(); // TODO
  auto envir = this;

  envir->AddComponent<Ground>();

  for (int x = -kLabyrinthRadius; x <= kLabyrinthRadius; ++x) {
    for (int z = -kLabyrinthRadius; z <= kLabyrinthRadius; ++z) {
      engine::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<Wall>(wall_transform);

      if ((abs(x) > 1 || abs(z) > 1) && x != kLabyrinthRadius && z != kLabyrinthRadius &&
          rand()%2 == 0) {
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
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall1.obj", wall_transform);
    }
  }

  for (int x = -kBorderRadius; x <= kBorderRadius; x += 2*kBorderRadius) {
    for (int z = -kBorderRadius; z <= kBorderRadius; ++z) {
      engine::Transform wall_transform;
      wall_transform.set_local_pos({x * kWallLength, -0.5, z * kWallLength});
      envir->AddComponent<BorderWall>("wall/bigwall2.obj", wall_transform);
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
  gl::BindToTexUnit(shadow_->shadow_texture(), engine::kShadowTextureSlot);

  // Bind single level of texture to image unit so we can write to it from shaders
  glBindImageTexture(engine::kVoxelTextureSlot, vct_->voxel_texture().expose(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
  vct_->VoxelizeStart();
  Scene::VoxelizeAll(vct_->uModelMatrix());
  vct_->VoxelizeEnd();
  glBindImageTexture(engine::kVoxelTextureSlot, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

  gl::BindToTexUnit(vct_->voxel_texture(), engine::kVoxelTextureSlot);

  Scene::RenderAll();
  // DebugVoxels(shader_manager(), vct_->voxel_dimensions(), vct_->voxel_grid_world_size()).Render(this);

  gl::UnbindFromTexUnit(vct_->voxel_texture(), engine::kVoxelTextureSlot);
  gl::UnbindFromTexUnit(shadow_->shadow_texture(), engine::kShadowTextureSlot);
#endif
}

void MainScene::KeyAction(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS && key == GLFW_KEY_F2) {
    auto eng = scene_->engine();
    eng->LoadScene(std::unique_ptr<engine::Scene>{new MainScene{eng, eng->window()}});
  }
}
