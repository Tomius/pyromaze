#version 330 core

out vec4 color;

uniform int uDimensions;

uniform sampler3D uVoxelsTexture;

void main() {
	vec3 pos; // Center of voxel
	pos.x = gl_VertexID % uDimensions;
	pos.z = (gl_VertexID / uDimensions) % uDimensions;
	pos.y = gl_VertexID / (uDimensions*uDimensions);

	color = texture(uVoxelsTexture, pos/uDimensions);
	gl_Position = vec4(pos - uDimensions*0.5, 1);
}
