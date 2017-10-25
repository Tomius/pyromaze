// Copyright (c) Tamas Csala

#version 330 core
#extension GL_ARB_bindless_texture : require

in vec3 c_vPos;
in vec3 w_vNormal;
in vec2 vTexCoord;

uniform uvec2 uDiffuseTextureId;

out vec4 fragColor;

void main() {
  vec3 fake_light_pos = normalize(vec3(0.4, 0.8, 0.2));
  float dot_value = dot(normalize(w_vNormal), fake_light_pos);
  vec3 fake_lighting = mix(vec3(0.09), vec3(0.9), (1 + dot_value)/2);

  vec4 color = texture2D(uDiffuseTextureId, vTexCoord);

  fragColor = vec4(color.rgb * fake_lighting, color.a);
}
