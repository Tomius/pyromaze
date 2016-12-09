// Copyright (c) Tamas Csala

#version 330 core

#include "lighting.frag"
#include "post_process.frag"

in vec3 w_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;
in vec3 w_vTangent;

uniform sampler2DShadow uShadowMap;
uniform sampler2D uDiffuseTexture;
uniform mat4 uShadowCP;

out vec4 fragColor;

// VCT ----------

uniform float uShowDiffuse = 1.0;
uniform float uShowIndirectDiffuse = 1.0;
uniform float uShowIndirectSpecular = 1.0;
uniform float uShowAmbientOcculision = 1.0;
uniform float uShowAmbientOcculisionOnly = 0.0;

uniform vec3 uCameraPos;

uniform sampler3D uVoxelTexture;
uniform float uVoxelDimensions = 128;
uniform float uVoxelGridWorldSize = 256;

const float MAX_DIST = 64.0;
const float ALPHA_THRESH = 0.95;

// 6 60 degree cone
const int NUM_CONES = 6;
vec3 coneDirections[6] = vec3[] (
    vec3(0, 1, 0),
    vec3(0, 0.5, 0.866025),
    vec3(0.823639, 0.5, 0.267617),
    vec3(0.509037, 0.5, -0.700629),
    vec3(-0.509037, 0.5, -0.700629),
    vec3(-0.823639, 0.5, 0.267617)
);
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);

// // 5 90 degree cones
// const int NUM_CONES = 5;
// vec3 coneDirections[5] = vec3[]
// (                            vec3(0, 1, 0),
//                             vec3(0, 0.707, 0.707),
//                             vec3(0, 0.707, -0.707),
//                             vec3(0.707, 0.707, 0),
//                             vec3(-0.707, 0.707, 0)
//                             );
// float coneWeights[5] = float[](0.28, 0.18, 0.18, 0.18, 0.18);

mat3 tangentToWorld;

vec4 sampleVoxels(vec3 worldPosition, float lod) {
    vec3 offset = vec3(1.0 / uVoxelDimensions, 1.0 / uVoxelDimensions, 0); // Why??
    vec3 voxelTextureUV = worldPosition / (uVoxelGridWorldSize * 0.5);
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5 + offset;
    return textureLod(uVoxelTexture, voxelTextureUV, lod);
}

// occlusion is for ambient occlusion
vec4 coneTrace(vec3 direction, float tanHalfAngle, out float occlusion) {
  float lod = 0.0;
  vec3 color = vec3(0);
  float alpha = 0.0;
  occlusion = 0.0;

  float voxelWorldSize = uVoxelGridWorldSize / uVoxelDimensions;
  float dist = voxelWorldSize; // Start one voxel away to avoid self occlusion
  vec3 startPos = w_vPos + normalize(w_vNormal) * voxelWorldSize; // Plus move away slightly in the normal direction to avoid
                                                                  // self occlusion in flat surfaces
  while (dist < MAX_DIST && alpha < ALPHA_THRESH) {
    // smallest sample diameter possible is the voxel size
    float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
    float lodLevel = log2(diameter / voxelWorldSize);
    vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);

    // front-to-back compositing
    color = /*alpha**/color + (1.0-alpha)*voxelColor.rgb;
    alpha += (1.0-alpha) * voxelColor.a;
    occlusion += 1.2 * (1.0-alpha) * voxelColor.a * (1.0 + 0.9*dist);
    dist += diameter * 0.5; // smoother than += diameter
  }

  occlusion = clamp(occlusion, 0, 1);
  return vec4(color, 1.0);
}

vec4 IndirectLight(out float occlusion_out) {
    vec4 color = vec4(0);
    occlusion_out = 0.0;

    for(int i = 0; i < NUM_CONES; i++) {
        float occlusion = 0.0;
        // 60 degree cones -> tan(30) = 0.577
        // 90 degree cones -> tan(45) = 1.0
        color += coneWeights[i] * coneTrace(tangentToWorld * coneDirections[i], 0.577, occlusion);
        occlusion_out += coneWeights[i] * occlusion;
    }

    return color;
}

// VCT ----------

void main() {
  vec4 materialColor = texture(uDiffuseTexture, vTexCoord);
  float alpha = materialColor.a;

  if (alpha < 0.5) {
    discard;
  }

  vec3 tangent = normalize(w_vTangent);
  vec3 normal = normalize(w_vNormal);
  tangentToWorld = inverse(transpose(mat3(tangent, normal, cross(tangent, normal))));

  // Calculate diffuse light
  vec3 diffuseReflection;
  float occlusion = 0.0;
  {
    // Shadow map
    vec4 shadow_coord = uShadowCP * vec4(w_vPos, 1.0);
    shadow_coord.xyz /= shadow_coord.w;
    shadow_coord.z -= 0.002;
    shadow_coord.xyz = (shadow_coord.xyz + 1) * 0.5;
    float visibility = texture(uShadowMap, shadow_coord.xyz);

    // Direct diffuse light
    vec3 directDiffuseLight = uShowDiffuse > 0.5 ? DiffuseLighting(w_vPos, normal, visibility < 0.5) : vec3(0.0);

    // Indirect diffuse light
    vec3 indirectDiffuseLight = IndirectLight(occlusion).rgb;
    indirectDiffuseLight = uShowIndirectDiffuse > 0.5 ? indirectDiffuseLight : vec3(0.0);
    occlusion = uShowAmbientOcculisionOnly > 0.5 || uShowAmbientOcculision > 0.5 ? occlusion : 1.0;

    // Sum direct and indirect diffuse light and tweak a little bit
    diffuseReflection = occlusion*(directDiffuseLight + indirectDiffuseLight) * materialColor.rgb;
  }

  // Calculate specular light
  vec3 specularReflection;
  {
    vec4 specularColor = materialColor * 0.5;
    vec3 reflectDir = reflect(normalize(uCameraPos - w_vPos), normal);

    // Maybe fix so that the cone doesnt trace below the plane defined by the surface normal.
    // For example so that the floor doesnt reflect itself when looking at it with a small angle
    float specularOcclusion;
    vec4 tracedSpecular = coneTrace(reflectDir, 0.07, specularOcclusion); // 0.2 = 22.6 degrees, 0.1 = 11.4 degrees, 0.07 = 8 degrees angle
    specularReflection = uShowIndirectSpecular > 0.5 ? specularColor.rgb * tracedSpecular.rgb * 0.75 : vec3(0.0);
  }

  vec3 linearHDRColor = diffuseReflection + specularReflection;
  vec3 finalColor = uShowAmbientOcculisionOnly > 0.5 ? vec3(occlusion) : PostProcess (linearHDRColor);
  fragColor = vec4(finalColor, alpha);
}
