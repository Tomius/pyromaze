#version 330 core

#include "post_process.frag"

in vec3 normal_world;
in vec4 fragColor;

out vec4 color;

void main() {
	if (fragColor == vec4(0.0))
		discard;

	color = vec4(PostProcess(fragColor.rgb) + 0.1, 1.0);
}
