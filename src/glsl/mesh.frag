// Copyright (c) Tamas Csala

#version 120

#include "lighting.frag"
#include "post_process.frag"

varying vec3 w_vPos;
varying vec3 w_vNormal;
varying vec2 vTexCoord;

uniform sampler2D uDiffuseTexture;

void main() {
  vec3 lighting = DiffuseLighting(w_vPos, normalize(w_vNormal), false);
  vec4 color = texture2D(uDiffuseTexture, vTexCoord);

  gl_FragColor = vec4(PostProcess(color.rgb * lighting), color.a);
}
