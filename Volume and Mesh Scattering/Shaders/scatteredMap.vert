#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 camera_matrix;

out VS_OUT{
	vec3 frag_pos;
	vec3 frag_normal;
} vs_out;

void main()
{
	vs_out.frag_pos = vec3(model_matrix * vec4(position, 1.0f));

	mat3 normalMatrix = transpose(inverse(mat3(model_matrix)));
	vs_out.frag_normal = normalMatrix * normal;
}