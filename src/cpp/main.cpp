// Copyright (c) Tamas Csala

#include "engine/game_engine.hpp"
#include "./main_scene.hpp"

int main(const int argc, const char *argv[]) {
  engine::GameEngine engine;
  engine.LoadScene(std::unique_ptr<engine::Scene>{new MainScene{&engine, engine.window()}});
  engine.Run();
}

