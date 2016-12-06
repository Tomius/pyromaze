#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout (triangles) in; // glDrawArrays is set to triangles so that's what we're working with
layout (triangle_strip, max_vertices = 3) out;

// Input from vertex shader, stored in an array
in vData {
  vec2 texCoord;
  vec4 shadow_position;
  vec3 normal;
} vertices[];

// Data that will be sent to fragment shader
out fData {
  vec3 w_pos;
  vec2 texCoord;
  vec4 shadow_position;
  vec3 normal;
  flat int axis;
} frag;

uniform mat4 uProjX;
uniform mat4 uProjY;
uniform mat4 uProjZ;

void main() {
  vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 normal = normalize(cross(p1,p2));

  float nDotX = abs(normal.x);
  float nDotY = abs(normal.y);
  float nDotZ = abs(normal.z);

  // 0 = x axis dominant, 1 = y axis dominant, 2 = z axis dominant
  frag.axis = (nDotX >= nDotY && nDotX >= nDotZ) ? 1 : (nDotY >= nDotX && nDotY >= nDotZ) ? 2 : 3;
  mat4 projectionMatrix = frag.axis == 1 ? uProjX : frag.axis == 2 ? uProjY : uProjZ;

  // For every vertex sent in vertices
  for (int i = 0; i < gl_in.length(); i++) {
    vec3 middlePos = gl_in[0].gl_Position.xyz / 3.0 + gl_in[1].gl_Position.xyz / 3.0 + gl_in[2].gl_Position.xyz / 3.0;
    frag.texCoord = vertices[i].texCoord;
    frag.shadow_position = vertices[i].shadow_position;
    frag.normal = vertices[i].normal;
    frag.w_pos = gl_in[i].gl_Position.xyz;
    gl_Position = projectionMatrix * gl_in[i].gl_Position;
    EmitVertex();
  }

  // Finished creating vertices
  EndPrimitive();
}
