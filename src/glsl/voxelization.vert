#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

uniform mat4 uShadowCP;
uniform mat4 uModelMatrix;

out vData {
  vec2 texCoord;
  vec4 shadow_position;
  vec3 normal;
} vert;

void main() {
  vert.texCoord = aTexCoord;
  vert.shadow_position = uShadowCP * vec4(aPosition, 1);
	vert.shadow_position.xyz = vert.shadow_position.xyz * 0.5f + 0.5f;
  vert.normal = normalize((uModelMatrix * vec4(aNormal,0)).xyz);

  gl_Position = uModelMatrix * vec4(aPosition, 1);
}
