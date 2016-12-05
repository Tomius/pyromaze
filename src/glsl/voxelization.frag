#version 330 core
#extension GL_ARB_shader_image_load_store : enable

// Data from geometry shader
in fData {
  vec2 texCoord;
  vec4 shadow_position; // Position from the shadow map point of view
  vec3 normal;
  flat int axis;
} frag;

uniform sampler2DShadow uShadowMap;
// uniform layout(RGBA8) image3D uVoxelTexture;
uniform layout(r32ui) uimage3D uVoxelTexture;
uniform sampler2D uDiffuseTexture;
uniform int uVoxelDimensions;
uniform vec3 uLightDirection = vec3(1, 1, 1);

out vec4 debugFragColor;

// Auxiliary functions borrowed from OpenGL Insights, 2011

uint convVec4ToRGBA8(vec4 val) {
  return (uint(val.w) & 0x000000FFu) << 24U
    | (uint(val.z) & 0x000000FFu) << 16U
    | (uint(val.y) & 0x000000FFu) << 8U
    | (uint(val.x) & 0x000000FFu);
}

vec4 convRGBA8ToVec4(uint val) {
  return vec4(float((val & 0x000000FFu)),
              float((val & 0x0000FF00u) >> 8U),
              float((val & 0x00FF0000u) >> 16U),
              float((val & 0xFF000000u) >> 24U));
}

uint encUnsignedNibble(uint m, uint n) {
  return (m & 0xFEFEFEFEu)
    | (n & 0x00000001u)
    | (n & 0x00000002u) << 7U
    | (n & 0x00000004u) << 14U
    | (n & 0x00000008u) << 21U;
}

uint decUnsignedNibble(uint m) {
  return (m & 0x00000001u)
    | (m & 0x00000100u) >> 7U
    | (m & 0x00010000u) >> 14U
    | (m & 0x01000000u) >> 21U;
}

void imageAtomicRGBA8Avg(layout(r32ui) uimage3D img,
                         ivec3 coords, vec4 val) {

  // LSBs are used for the sample counter of the moving average.

  val *= 255.0;
  uint newVal = encUnsignedNibble(convVec4ToRGBA8(val), 1u);
  uint prevStoredVal = 0u;
  uint currStoredVal;

  int counter = 0;
  // Loop as long as destination value gets changed by other threads
  while ((currStoredVal = imageAtomicCompSwap(img, coords, prevStoredVal, newVal))
         != prevStoredVal && counter < 16) {

    vec4 rval = convRGBA8ToVec4(currStoredVal & 0xFEFEFEFEu);
    uint n = decUnsignedNibble(currStoredVal);
    rval = rval * n + val;
    rval /= ++n;
    rval = round(rval / 2) * 2;
    newVal = encUnsignedNibble(convVec4ToRGBA8(rval), n);

    prevStoredVal = currStoredVal;

    counter++;
  }
}

void main() {
  vec4 materialColor = texture(uDiffuseTexture, frag.texCoord);
  float cosTheta = 0.1 * max(0.0, dot(normalize(frag.normal), uLightDirection));

  // Do shadow map lookup here
  // TODO: Splat photons onto the voxels at a later stage using a separate shader
  float visibility = texture(uShadowMap, vec3(frag.shadow_position.xy, (frag.shadow_position.z - 0.001)/frag.shadow_position.w));
  visibility = max(visibility, 0.12);

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

  vec4 colorToStore = vec4(cosTheta * visibility * materialColor.rgb, 1.0);
  // imageStore(uVoxelTexture, texPos, colorToStore);
  imageAtomicRGBA8Avg(uVoxelTexture, texPos, colorToStore);

  debugFragColor = vec4(vec3(texPos)/uVoxelDimensions, 1);
}
