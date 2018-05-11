#version 330

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;

in vec3 in_coord;
out vec4 color;

void main() {	

	float value = texture3D(volume_text, in_coord).r;
 	vec4 actual_color = texture1D(transfer_function_text, value);
 	// previus algo
}