// Copyright (c), Tamas Csala

#version 330 core

#export vec4 textureBicubic(sampler2D tex, vec2 texCoords);
#export float textureBicubic(sampler2DArrayShadow tex, vec4 texCoords);

vec4 cubic(float v) {
  vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
  vec4 s = n * n * n;
  float x = s.x;
  float y = s.y - 4.0 * s.x;
  float z = s.z - 4.0 * s.y + 6.0 * s.x;
  float w = 6.0 - x - y - z;
  return vec4(x, y, z, w) * (1.0/6.0);
}

void GetSamplingParams(vec2 texSize, vec2 texCoords, out vec4 offset, out vec4 s) {
  vec2 invTexSize = 1.0 / texSize;

  texCoords = texCoords * texSize - 0.5;
  vec2 fxy = fract(texCoords);
  texCoords -= fxy;

  vec4 xcubic = cubic(fxy.x);
  vec4 ycubic = cubic(fxy.y);

  vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;

  s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
  offset = c + vec4(xcubic.yw, ycubic.yw) / s;
  offset *= invTexSize.xxyy;
}

vec4 GetResultColor(vec4 sample0, vec4 sample1, vec4 sample2, vec4 sample3, vec4 s) {
  float sx = s.x / (s.x + s.y);
  float sy = s.z / (s.z + s.w);

  return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

float GetResultColor(float sample0, float sample1, float sample2, float sample3, vec4 s) {
  float sx = s.x / (s.x + s.y);
  float sy = s.z / (s.z + s.w);

  return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

vec4 textureBicubic(sampler2D tex, vec2 texCoords) {
  vec4 offset, s;
  GetSamplingParams(textureSize(tex, 0), texCoords, offset, s);

  vec4 sample0 = texture(tex, offset.xz);
  vec4 sample1 = texture(tex, offset.yz);
  vec4 sample2 = texture(tex, offset.xw);
  vec4 sample3 = texture(tex, offset.yw);

  return GetResultColor(sample0, sample1, sample2, sample3, s);
}

float textureBicubic(sampler2DArrayShadow tex, vec4 texCoords) {
  vec4 offset, s;
  GetSamplingParams(textureSize(tex, 0).xy, texCoords.xy, offset, s);

  float sample0 = texture(tex, vec4(offset.xz, texCoords.zw));
  float sample1 = texture(tex, vec4(offset.yz, texCoords.zw));
  float sample2 = texture(tex, vec4(offset.xw, texCoords.zw));
  float sample3 = texture(tex, vec4(offset.yw, texCoords.zw));

  return GetResultColor(sample0, sample1, sample2, sample3, s);
}

