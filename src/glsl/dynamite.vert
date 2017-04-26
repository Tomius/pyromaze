// Copyright (c) Tamas Csala

#version 330 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;
uniform vec3 uFirePos;

out vec3 w_vPos;
out vec3 w_vNormal;
out vec2 vTexCoord;
out vec3 w_vTangent;

void main() {
  mat3 normalMatrix = inverse(mat3(uModelMatrix));
  w_vNormal = aNormal * normalMatrix;
  w_vTangent = aTangent * normalMatrix;
  vTexCoord = aTexCoord;

  bool fuse_already_burnt = aPosition.x > uFirePos.x && aPosition.y > 1.15;
  vec4 m_pos = fuse_already_burnt ? vec4(uFirePos, 1) : aPosition;
  w_vPos = vec3(uModelMatrix * m_pos);

  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * m_pos;
}
