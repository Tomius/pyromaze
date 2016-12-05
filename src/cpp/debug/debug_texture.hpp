// Copyright (c) Tamas Csala

#ifndef DEBUG_TEXTURE_HPP_
#define DEBUG_TEXTURE_HPP_

#include <oglwrap/shader.h>
#include <oglwrap/uniform.h>
#include <oglwrap/shapes/rectangle_shape.h>
#include <oglwrap/textures/texture_2D.h>
#include <oglwrap/smart_enums.h>

#include "engine/shader_manager.hpp"

class DebugTexture {
  gl::RectangleShape rect_;
  engine::ShaderProgram prog_;

public:
  DebugTexture(engine::ShaderManager* shader_manager)
      : rect_({gl::RectangleShape::kPosition, gl::RectangleShape::kTexCoord})
      , prog_(shader_manager->get("debug_texture.vert"),
              shader_manager->get("debug_texture.frag")) {

    gl::Use(prog_);
    gl::UniformSampler(prog_, "uTex").set(0);
    (prog_ | "aPosition").bindLocation(rect_.kPosition);
    (prog_ | "aTexCoord").bindLocation(rect_.kTexCoord);
    prog_.validate();
    gl::Unuse(prog_);
  }

  void Render(const gl::Texture2D& tex) {
    gl::Use(prog_);
    gl::BindToTexUnit(tex, 0);

    gl::TemporarySet capabilies{{{gl::kCullFace, false},
                                 {gl::kDepthTest, false}}};

    rect_.render();
    gl::Unbind(tex);
    gl::Unuse(prog_);
  }
};

#endif  // DEBUG_TEXTURE_HPP_
