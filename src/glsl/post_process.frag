// Copyright (c) 2016, Tamas Csala

#version 120

#export vec3 PostProcess(vec3 color);

float Luminance(vec3 c) {
  return sqrt(0.299 * c.r*c.r + 0.587 * c.g*c.g + 0.114 * c.b*c.b);
}

vec3 ToneMap_Internal(vec3 x) {
  float A = 0.22;
  float B = 0.30;
  float C = 0.10;
  float D = 0.20;
  float E = 0.01;
  float F = 0.30;

  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

vec3 ToneMap(vec3 color) {
  float oldLuminance = Luminance(color);
  if (oldLuminance < 1e-3) {
    return color;
  }

  vec3 newColor = ToneMap_Internal(color) / ToneMap_Internal(vec3(11.2));
  float newLuminance = Luminance (newColor);
  float ratio = newLuminance / oldLuminance;
  return color * ratio;
}

vec3 PostProcess(vec3 color) {
  return sqrt(ToneMap(color));
}
