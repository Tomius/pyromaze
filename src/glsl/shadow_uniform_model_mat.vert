// Copyright (c) Tamas Csala

#version 330

layout(location = 0) in vec4 aPosition;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;

void main() {
  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * aPosition;
}
