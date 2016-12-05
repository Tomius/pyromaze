// Copyright (c) Tamas Csala

#version 330 core

in vec2 vTexCoord;

uniform sampler2D uTex;

out vec4 fragColor;

void main() {
  // fragColor = vec4(texture2D(uTex, vTexCoord).rrr, 1);
  fragColor = vec4(texture2D(uTex, vec2(vTexCoord.x, 1-vTexCoord.y)).rgb, 1);
}
