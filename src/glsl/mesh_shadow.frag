// Copyright (c) Tamas Csala

#version 120

#include "lighting.frag"
#include "post_process.frag"

varying vec3 w_vPos;
varying vec3 w_vNormal;
varying vec2 vTexCoord;

uniform sampler2DShadow uShadowMap;
uniform sampler2D uDiffuseTexture;
uniform mat4 uShadowCP;

void main() {
  vec4 color = texture2D(uDiffuseTexture, vTexCoord);

  vec4 shadow_coord = uShadowCP * vec4(w_vPos, 1.0);
  shadow_coord.xyz /= shadow_coord.w;
  shadow_coord.z -= 0.002;
  shadow_coord.xyz = (shadow_coord.xyz + 1) * 0.5;
  float visibility = shadow2D(uShadowMap, shadow_coord.xyz).r;

  vec3 lighting = DiffuseLighting(w_vPos, normalize(w_vNormal), visibility < 0.5);

  gl_FragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
