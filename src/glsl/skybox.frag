// Copyright (c) 2016, Tamas Csala

#version 130

in vec3 vDirection;

uniform samplerCube tex;

out vec4 fragColor;

void main() {
  fragColor = texture(tex, normalize(vec3(vDirection.x, vDirection.y, vDirection.z)));
}
