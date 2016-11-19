// Copyright (c) 2016, Tamas Csala

#version 120

varying vec3 c_vPos;
varying vec3 w_vNormal;
varying vec2 vTexCoord;

uniform sampler2D uDiffuseTexture;

void main() {
  vec3 fake_light_pos = normalize(vec3(0.4, 0.8, 0.2));
  float dot_value = dot(normalize(w_vNormal), fake_light_pos);
  vec3 fake_lighting = mix(vec3(0.09), vec3(0.9), (1 + dot_value)/2);

  vec4 color = texture2D(uDiffuseTexture, vTexCoord);

  gl_FragColor = vec4(color.rgb * fake_lighting, color.a);
}
