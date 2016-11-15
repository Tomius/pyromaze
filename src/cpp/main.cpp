// Copyright (c) 2016, Tamas Csala

#define OGLWRAP_INSTANTIATE 1
#include "engine/game_engine.hpp"
#include "./main_scene.hpp"

int main(const int argc, const char *argv[]) {
  engine::GameEngine engine;
  engine.LoadScene(make_unique<MainScene>(engine.window(), engine.shader_manager()));
  engine.Run();
}

