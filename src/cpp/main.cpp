// Copyright (c) Tamas Csala

#include <Silice3D/core/game_engine.hpp>

#include "./main_scene.hpp"

int main(const int argc, const char *argv[]) {
  Silice3D::GameEngine engine("Pyromaze", Silice3D::GameEngine::WindowMode::kFullScreen);
  engine.LoadScene(std::unique_ptr<Silice3D::Scene>{new MainScene{&engine}});
  engine.Run();
}

