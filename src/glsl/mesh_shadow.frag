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
  vec3 diffuse_color = texture(uDiffuseTexture, vTexCoord).rgb;
  vec3 output_color = Silice3D_CalculateLighting(w_vPos, normalize(w_vNormal), true,
                                                 diffuse_color, diffuse_color, 16);
  fragColor = vec4(PostProcess(output_color), 1.0);
}
