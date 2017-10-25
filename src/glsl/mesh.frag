// Copyright (c) Tamas Csala

#version 330 core
#extension GL_ARB_bindless_texture : require

#include "lighting.frag"
#include "post_process.frag"
#include "bicubic_sampling.glsl"

in vec3 w_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;

out vec4 fragColor;

uniform uvec2 uDiffuseTextureId;

void main() {
  vec3 lighting = CalculateLighting(w_vPos, normalize(w_vNormal), 1.0);
  vec4 color = texture(sampler2D(uDiffuseTextureId), vTexCoord);

  fragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
