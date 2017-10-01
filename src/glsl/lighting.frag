// Copyright (c) Tamas Csala

#version 120

#export vec3 DiffuseLighting(vec3 position, vec3 normal, float in_shadow);

struct DirectionalLightSource {
  vec3 direction, color;
};

struct PointLightSource {
  vec3 position, color;
};

#define MAX_LIGHTS 100
uniform DirectionalLightSource uDirectionalLights[MAX_LIGHTS];
uniform int uDirectionalLightCount;

uniform PointLightSource uPointLights[MAX_LIGHTS];
uniform int uPointLightCount;

vec3 DiffuseLighting(vec3 position, vec3 normal, float in_shadow) {
  vec3 sum_lighting = vec3(0.0);
  float shadow_mult = (in_shadow*0.95 + 0.05);
  for(int i = 0; i < uDirectionalLightCount; ++i) {
    sum_lighting += shadow_mult * max(dot(normal, uDirectionalLights[i].direction), 0) * uDirectionalLights[i].color;
  }
  for(int i = 0; i < uPointLightCount; ++i) {
    vec3 toLight = uPointLights[i].position-position;
    sum_lighting += max(dot(normal, toLight), 0) * uPointLights[i].color / (0.01 + dot(toLight, toLight));
  }
  return sum_lighting + 0.02;
}
