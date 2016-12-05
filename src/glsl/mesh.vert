// Copyright (c) Tamas Csala

#version 330 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;
uniform mat3 uNormalMatrix;

out vec3 w_vPos;
out vec3 w_vNormal;
out vec2 vTexCoord;
out vec3 w_vTangent;

void main() {
  w_vNormal = aNormal * uNormalMatrix;
  w_vTangent = aTangent * uNormalMatrix;
  vTexCoord = aTexCoord;
  w_vPos = vec3(uModelMatrix * aPosition);
  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * aPosition;
}
