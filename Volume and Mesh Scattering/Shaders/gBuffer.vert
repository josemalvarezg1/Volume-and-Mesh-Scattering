#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT
{
	vec3 frag_pos;
	vec3 frag_normal;
} vs_out;

uniform mat4 model_matrix;

void main()
{
	vs_out.frag_pos = vec3(model_matrix * vec4(position, 1.0f));
	vs_out.frag_normal = transpose(inverse(mat3(model_matrix))) * normal;
}