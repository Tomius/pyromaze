// Copyright (c) Tamas Csala

#version 120

attribute vec4 aPosition;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;

void main() {
  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * aPosition;
}
