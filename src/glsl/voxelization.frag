#version 330 core
#extension GL_ARB_shader_image_load_store : enable

#include "lighting.frag"

// Data from geometry shader
in fData {
  vec3 w_pos;
  vec2 texCoord;
  vec4 shadow_position; // Position from the shadow map point of view
  vec3 normal;
  flat int axis;
} frag;

uniform sampler2DShadow uShadowMap;
uniform layout(r32ui) uimage3D uVoxelTexture;
uniform sampler2D uDiffuseTexture;
uniform int uVoxelDimensions;
uniform vec3 uLightDirection = vec3(1, 1, 1);

out vec4 debugFragColor;

// Auxiliary functions borrowed from OpenGL Insights, 2011

uint convVec4ToRGBA8(vec4 val) {
  return (uint(val.w) & 0x000000FFU) << 24U
    | (uint(val.z) & 0x000000FFU) << 16U
    | (uint(val.y) & 0x000000FFU) << 8U
    | (uint(val.x) & 0x000000FFU);
}

uint MyPackUnorm4x8(vec4 val) {
  return convVec4ToRGBA8(round(clamp(val, vec4(0.0), vec4(1.0)) * 255.0));
}

vec4 convRGBA8ToVec4(uint val) {
  return vec4(float((val & 0x000000FFU)),
              float((val & 0x0000FF00U) >> 8U),
              float((val & 0x00FF0000U) >> 16U),
              float((val & 0xFF000000U) >> 24U));
}

vec4 MyUnpackUnorm4x8(uint val) {
  return convRGBA8ToVec4(val) / 255.0;
}

vec3 imageAtomicAverageRGBA8(layout(r32ui) coherent volatile uimage3D voxels, ivec3 coord, vec3 nextVec3) {
  uint nextUint = MyPackUnorm4x8(vec4(nextVec3,1.0/255.0));
  uint prevUint = 0U;
  uint currUint;

  vec4 currVec4;

  vec3 average;
  uint count;

  //"Spin" while threads are trying to change the voxel
  while((currUint = imageAtomicCompSwap(voxels, coord, prevUint, nextUint)) != prevUint)
  {
    prevUint = currUint;                    //store packed rgb average and count
    currVec4 = MyUnpackUnorm4x8(currUint);    //unpack stored rgb average and count

    average = currVec4.rgb;             //extract rgb average
    count   = uint(currVec4.a*255.0);  //extract count

    //Compute the running average
    average = (average*count + nextVec3) / (count+1U);

    //Pack new average and incremented count back into a uint
    nextUint = MyPackUnorm4x8(vec4(average, (count+1U)/255.0f));
  }

  return average;
}

void main() {
  vec4 materialColor = texture(uDiffuseTexture, frag.texCoord);
  // float cosTheta = 0.2 * max(0.0, dot(normalize(frag.normal), uLightDirection));

  // Do shadow map lookup here
  // TODO: Splat photons onto the voxels at a later stage using a separate shader
  float visibility = texture(uShadowMap, vec3(frag.shadow_position.xy, (frag.shadow_position.z - 0.001)/frag.shadow_position.w));
  visibility = max(visibility, 0.12);

  vec3 lighting = DiffuseLighting(frag.w_pos, normalize(frag.normal), visibility < 0.5);

	ivec3 camPos = ivec3(gl_FragCoord.x, gl_FragCoord.y, uVoxelDimensions * gl_FragCoord.z);
	ivec3 texPos;
	if (frag.axis == 1) {
	  texPos.x = uVoxelDimensions - camPos.z;
    texPos.y = camPos.y;
		texPos.z = camPos.x;
	} else if (frag.axis == 2) {
		texPos.x = camPos.x;
    texPos.z = camPos.y;
    texPos.y = uVoxelDimensions - camPos.z;
	} else {
	  texPos = camPos;
	}

	// Flip it!
	texPos.z = uVoxelDimensions - texPos.z - 1;

  vec3 colorToStore = lighting * materialColor.rgb;
  vec3 avgColor = imageAtomicAverageRGBA8(uVoxelTexture, texPos, colorToStore);
  memoryBarrier();
  uint prev = 0U;
  imageAtomicCompSwap(uVoxelTexture, texPos, prev, MyPackUnorm4x8(vec4(avgColor, 1)));

  debugFragColor = vec4(vec3(texPos)/uVoxelDimensions, 1);
}
