// Copyright (c) Tamas Csala

#version 330 core
#extension GL_ARB_bindless_texture : require

#include "bicubic_sampling.glsl"

#export vec3 CalculateLighting(vec3 position, vec3 normal);

#define kMaxCascadesCount 4
#define DEBUG_VISUALIZATION_OF_CASCADES 0

struct DirectionalLightSource {
  vec3 direction, color;
  uvec2 shadowMapId; // sampler2DArrayShadow
  mat4 shadowCP[kMaxCascadesCount];
  int cascades_count;
};

struct PointLightSource {
  vec3 position, color;
};

#define MAX_DIR_LIGHTS 16 // TODO: Uniform buffer
uniform DirectionalLightSource uDirectionalLights[MAX_DIR_LIGHTS];
uniform int uDirectionalLightCount;

#define MAX_POINT_LIGHTS 128 // TODO: Uniform buffer
uniform PointLightSource uPointLights[MAX_POINT_LIGHTS];
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

vec4 GetShadowCoord(vec3 position, int lightNum, int selected_cascade) {
  vec4 shadow_coord = uDirectionalLights[lightNum].shadowCP[selected_cascade] * vec4(position, 1.0);
  shadow_coord.xyz /= shadow_coord.w;
  shadow_coord.z -= 0.00007 * pow(2.1, selected_cascade);
  shadow_coord.xy = (shadow_coord.xy + 1) * 0.5;
  return vec4(shadow_coord.xy, selected_cascade, shadow_coord.z);
}

vec3 CalculateLighting(vec3 position, vec3 normal) {
  vec3 sum_lighting = vec3(0.0);
  const float kShininess = 16;
  float kAmbientPower = 0.05;

  for (int i = 0; i < min(uDirectionalLightCount, MAX_DIR_LIGHTS); ++i) {
    vec3 light_dir = uDirectionalLights[i].direction;

    float diffuse_power = GetDiffusePower(normal, light_dir);
    float specular_power = GetSpecularPower(position, normal, light_dir, kShininess);

    int selected_cascade = 0;
    float shadow_mult = 1.0;
    int cascades_count = uDirectionalLights[i].cascades_count;
    if (cascades_count > 0) {
      // Visibility
      float alpha = 0.0;
      for (int j = 0; j < cascades_count; ++j) {
        vec4 shadow_coord = uDirectionalLights[i].shadowCP[j] * vec4(position, 1.0);
        shadow_coord.xyz /= shadow_coord.w;
        float max_coord = max(max(abs(shadow_coord.x), abs(shadow_coord.y)), shadow_coord.z);
        if (i == cascades_count - 1 || (max_coord < 1.0 && 0.0 < shadow_coord.z)) {
          selected_cascade = j;
          alpha = smoothstep(0.8, 1.0, max_coord);
          break;
        }
      }

      sampler2DArrayShadow shadowMap = sampler2DArrayShadow(uDirectionalLights[i].shadowMapId);
      float current_visibility = texture(shadowMap, GetShadowCoord (position, i, selected_cascade));
      float next_visibility = texture(shadowMap, GetShadowCoord (position, i, min(selected_cascade+1, cascades_count-1)));
      float visibility = mix(current_visibility, next_visibility, alpha);
      shadow_mult = (visibility*0.95 + 0.05);
    }

    vec3 lightColor = uDirectionalLights[i].color;
    #if DEBUG_VISUALIZATION_OF_CASCADES
    switch (selected_cascade) {
      case 0: lightColor = vec3(1, 0, 0); break;
      case 1: lightColor = vec3(0, 1, 0); break;
      case 2: lightColor = vec3(0, 0, 1); break;
      case 3: lightColor = vec3(1, 1, 0); break;
      case 4: lightColor = vec3(0, 1, 1); break;
    }
    #endif

    sum_lighting += (kAmbientPower + shadow_mult*(diffuse_power + specular_power)) * lightColor;
  }

  for (int i = 0; i < min(uPointLightCount, MAX_POINT_LIGHTS); ++i) {
    vec3 light_dir = uPointLights[i].position-position;

    float diffuse_power = GetDiffusePower(normal, light_dir);
    float specular_power = GetSpecularPower(position, normal, light_dir, kShininess);

    float attenuation = 1.0 / (0.01 + dot(light_dir, light_dir));

    sum_lighting += attenuation * (kAmbientPower + diffuse_power + specular_power) * uPointLights[i].color;
  }

  return sum_lighting;
}
