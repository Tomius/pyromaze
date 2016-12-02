// Copyright (c) Tamas Csala

#version 120

varying vec2 vTexCoord;

uniform sampler2D uTex;

void main() {
  gl_FragColor = vec4(texture2D(uTex, vTexCoord).rrr, 1);
}
