// Copyright (c) Tamas Csala

#version 330

layout(location = 0) in vec4 aPosition;
layout(location = 8) in mat4 aModelMatrix;

uniform mat4 uProjectionMatrix, uCameraMatrix;

void main() {
  gl_Position = uProjectionMatrix * uCameraMatrix * aModelMatrix * aPosition;
}
