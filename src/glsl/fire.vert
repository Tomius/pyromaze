// Copyright (c) Tamas Csala

#version 330 core

in vec3 aPosition;
in vec3 aNormal;

uniform mat4 uCameraMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;

out vec3 vNormal;

void main() {
  vNormal = aNormal;
  gl_Position = uProjectionMatrix * (uCameraMatrix * (uModelMatrix * vec4(aPosition, 1)));
}
