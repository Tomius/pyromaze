// Copyright (c) 2016, Tamas Csala

#version 120

attribute vec4 aPosition;
attribute vec2 aTexCoord;
attribute vec3 aNormal;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;
uniform mat3 uNormalMatrix;

varying vec3 w_vPos;
varying vec3 w_vNormal;
varying vec2 vTexCoord;

void main() {
  w_vNormal = aNormal * uNormalMatrix;
  vTexCoord = aTexCoord;
  w_vPos = vec3(uModelMatrix * aPosition);
  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * aPosition;
}
