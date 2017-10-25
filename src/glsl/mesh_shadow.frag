// Copyright (c) Tamas Csala

#version 330 core
#extension GL_ARB_bindless_texture : require

#include "lighting.frag"
#include "post_process.frag"
#include "bicubic_sampling.glsl"

in vec3 w_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;

uniform uvec2 uShadowMapId;
uniform uvec2 uDiffuseTextureId;
#define kCascadesCount 3
uniform mat4 uShadowCP[kCascadesCount];

#define DEBUG_VISUALIZATION_OF_CASCADES 0

out vec4 fragColor;

vec4 GetShadowCoord(int cascade_num) {
  vec4 shadow_coord = uShadowCP[cascade_num] * vec4(w_vPos, 1.0);
  shadow_coord.xyz /= shadow_coord.w;
  shadow_coord.z -= 0.002;
  shadow_coord.xyz = (shadow_coord.xyz + 1) * 0.5;
  return vec4(shadow_coord.xy, cascade_num, shadow_coord.z);
}

void main() {
  vec4 color = textureBicubic(sampler2D(uDiffuseTextureId), vTexCoord);

  int cascade_num = 0;
  float alpha = 0.0;
  const float kShadowBorder = 1.0; // border of shadow coordinates' utilized area
  const float kBlendingStart = 0.8;
  for (int i = 0; i < kCascadesCount; ++i) {
    vec4 shadow_coord = uShadowCP[i] * vec4(w_vPos, 1.0);
    shadow_coord.xyz /= shadow_coord.w;
    float max_coord = max(max(abs(shadow_coord.x), abs(shadow_coord.y)), abs(shadow_coord.z));
    if (i == kCascadesCount - 1 || max_coord < kShadowBorder) {
      cascade_num = i;
      alpha = clamp((max_coord - kBlendingStart) / (kShadowBorder - kBlendingStart), 0.0, 1.0);
      break;
    }
  }

  int next_cascade_num = min(cascade_num+1, kCascadesCount - 1);
  int next_next_cascade_num = min(cascade_num+2, kCascadesCount - 1);

  // if two cascade nums are the same, then the texture calls will use the exact same
  // shadowCoord, so the fetched texel will come from cache, which is practically for free
  sampler2DArrayShadow shadowMap = sampler2DArrayShadow(uShadowMapId);
  float current_visibility = textureBicubic(shadowMap, GetShadowCoord (cascade_num));
  float next_visibility = textureBicubic(shadowMap, GetShadowCoord (next_cascade_num));
  float next_next_visibility = textureBicubic(shadowMap, GetShadowCoord (next_next_cascade_num));
  float visibility = mix(current_visibility*next_visibility, next_visibility*next_next_visibility, alpha);

  vec3 lighting = CalculateLighting(w_vPos, normalize(w_vNormal), visibility);

#if DEBUG_VISUALIZATION_OF_CASCADES
  switch (cascade_num) {
    case 0: color = vec4(1, 0, 0, 1); break;
    case 1: color = vec4(0, 1, 0, 1); break;
    case 2: color = vec4(0, 0, 1, 1); break;
    case 3: color = vec4(1, 1, 0, 1); break;
    case 4: color = vec4(0, 1, 1, 1); break;
  }
#endif

  fragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
