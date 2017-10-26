// Copyright (c) Tamas Csala

#version 330 core

#include "lighting.frag"
#include "post_process.frag"
#include "bicubic_sampling.glsl"

in vec3 w_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;

uniform sampler2D uDiffuseTexture;

out vec4 fragColor;

void main() {
  vec4 color = texture(uDiffuseTexture, vTexCoord);
  vec3 lighting = CalculateLighting(w_vPos, normalize(w_vNormal));
  fragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
