// Copyright (c) 2016, Tamas Csala

#version 130

in vec3 vDirection;

uniform float uLifeTime;

out vec4 fragColor;

void main() {
  if (uLifeTime < 0.5) {
    fragColor = mix(vec4(1, 1, 0, 1), vec4(1, 0, 0, 1), 2*uLifeTime);
  } else {
    fragColor = mix(vec4(1, 0, 0, 1), vec4(0, 0, 0, 1), min(uLifeTime-0.5, 1));
  }
}
