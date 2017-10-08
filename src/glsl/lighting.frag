// Copyright (c) Tamas Csala

#version 120

#export vec3 CalculateLighting(vec3 position, vec3 normal, float visibility);

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

uniform vec3 w_uCamPos;

float GetDiffusePower(vec3 normal, vec3 light_dir) {
  return max(dot(normal, light_dir), 0);
}

float GetSpecularPower(vec3 position, vec3 normal, vec3 light_dir, float shininess) {
  vec3 view_vector = normalize(w_uCamPos - position);
  vec3 half_vector = normalize(light_dir + view_vector);
  return pow(max(dot(half_vector, normal), 0.0f), shininess);
}

vec3 CalculateLighting(vec3 position, vec3 normal, float visibility) {
  vec3 sum_lighting = vec3(0.0);
  float shadow_mult = (visibility*0.95 + 0.05);
  const float kShininess = 16;
  const float kAmbientPower = 0.2;

  for (int i = 0; i < uDirectionalLightCount; ++i) {
    vec3 light_dir = uDirectionalLights[i].direction;

    float diffuse_power = GetDiffusePower(normal, light_dir);
    float specular_power = GetSpecularPower(position, normal, light_dir, kShininess);

    sum_lighting += (kAmbientPower + shadow_mult*(diffuse_power + specular_power)) * uDirectionalLights[i].color;
  }

  for (int i = 0; i < uPointLightCount; ++i) {
    vec3 light_dir = uPointLights[i].position-position;

    float diffuse_power = GetDiffusePower(normal, light_dir);
    float specular_power = GetSpecularPower(position, normal, light_dir, kShininess);

    float attenuation = 1.0 / (0.01 + dot(light_dir, light_dir));

    sum_lighting += attenuation * (kAmbientPower + diffuse_power + specular_power) * uPointLights[i].color;
  }

  return sum_lighting;
}
