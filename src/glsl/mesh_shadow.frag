// Copyright (c) Tamas Csala

#version 130

#include "lighting.frag"
#include "post_process.frag"

in vec3 w_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;

uniform sampler2DArrayShadow uShadowMap;
uniform sampler2D uDiffuseTexture;
#define kCascadesCount 4
uniform mat4 uShadowCP[kCascadesCount];

#define DEBUG_VISUALIZATION_OF_CASCADES 0

out vec4 fragColor;

void main() {
  vec4 color = texture2D(uDiffuseTexture, vTexCoord);

  int layer = 0;
  vec4 shadow_coord = vec4(0.0);
  const float kShadowBorder = 0.9; // border of shadow coordinates utilazed area
  for (int i = 0; i < kCascadesCount; ++i) {
    shadow_coord = uShadowCP[i] * vec4(w_vPos, 1.0);
    shadow_coord.xyz /= shadow_coord.w;
    if (i == kCascadesCount - 1 || (abs(shadow_coord.x) < kShadowBorder &&
                                    abs(shadow_coord.y) < kShadowBorder &&
                                    abs(shadow_coord.z) < kShadowBorder)) {
      layer = i;
      break;
    }
  }

  shadow_coord.z -= 0.001;
  shadow_coord.xyz = (shadow_coord.xyz + 1) * 0.5;
  float visibility = texture(uShadowMap, vec4(shadow_coord.xy, layer, shadow_coord.z));

  vec3 lighting = DiffuseLighting(w_vPos, normalize(w_vNormal), visibility);

#if DEBUG_VISUALIZATION_OF_CASCADES
  switch (layer) {
    case 0: color = vec4(1, 0, 0, 1); break;
    case 1: color = vec4(0, 1, 0, 1); break;
    case 2: color = vec4(0, 0, 1, 1); break;
    case 3: color = vec4(1, 1, 0, 1); break;
    case 4: color = vec4(0, 1, 1, 1); break;
  }
#endif

  fragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
