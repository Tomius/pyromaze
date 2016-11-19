// Copyright (c) 2016, Tamas Csala

#version 130

in vec3 aPosition;

uniform mat4 uCameraMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;

out vec3 vDirection;

void main() {
  vDirection = aPosition.xyz;
  gl_Position = uProjectionMatrix * (uCameraMatrix * (uModelMatrix * vec4(aPosition, 1)));
}
