// Copyright (c) 2016, Tamas Csala

#version 120

attribute vec4 aPosition;
attribute vec2 aTexCoord;
attribute vec3 aNormal;

uniform mat4 uProjectionMatrix, uCameraMatrix, uModelMatrix;
uniform mat3 uNormalMatrix;
uniform vec3 uFirePos;

varying vec3 w_vPos;
varying vec3 w_vNormal;
varying vec2 vTexCoord;

void main() {
  w_vNormal = aNormal * uNormalMatrix;
  vTexCoord = aTexCoord;

  bool fuse_already_burnt = aPosition.x > uFirePos.x && aPosition.y > 1.15;
  vec4 m_pos = fuse_already_burnt ? vec4(uFirePos, 1) : aPosition;
  w_vPos = vec3(uModelMatrix * m_pos);

  gl_Position = uProjectionMatrix * uCameraMatrix * uModelMatrix * m_pos;
}
