// Copyright (c) Tamas Csala

#version 330 core

in vec3 vNormal;

uniform float uLifeTime;

out vec4 fragColor;

void main() {
  vec3 fake_light_pos = normalize(vec3(0.4, 0.8, 0.2));
  float dot_value = dot(normalize(vNormal), fake_light_pos);
  vec4 fake_lighting = vec4(mix(vec3(0.5), vec3(1.0), (1 + dot_value)/2), 1.0);
  if (uLifeTime < 0.5) {
    fragColor = mix(vec4(1, 1, 0, 1), vec4(1, 0, 0, 1), 2*uLifeTime) * fake_lighting;
  } else {
    fragColor = mix(vec4(1, 0, 0, 1), vec4(0, 0, 0, 1), min(uLifeTime-0.5, 1)) * fake_lighting;
  }
}
