// Copyright (c) 2016, Tamas Csala

#version 130

in vec3 aPosition;

uniform mat3 uCameraMatrix;
uniform mat4 uProjectionMatrix;

out vec3 vDirection;

void main() {
  vDirection = aPosition;
  gl_Position = uProjectionMatrix * vec4(uCameraMatrix * 10 * aPosition, 1);
}
