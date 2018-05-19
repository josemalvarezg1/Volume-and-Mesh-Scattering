#version 330

layout (location = 0) out vec4 color;

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;

in vec3 in_coord;
in vec3 frag_pos;
in float ray_distance;

void main() 
{
	float value;
	vec3 ray_direction, intersection_point;
	vec4 accumulated_color, actual_color, offset;

	value = texture(volume_text, in_coord).x;
	actual_color = texture(transfer_function_text, value);
	color = actual_color;
}