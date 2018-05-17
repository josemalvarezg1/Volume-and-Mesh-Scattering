#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat4 model_matrix;

out vec3 frag_pos;
out vec3 frag_normal;
out vec3 frag_light_pos;

void main()
{
	frag_pos = vec3(model_matrix * vec4(position, 1.0f));
	frag_light_pos = vec3(0.0f, 10.0f, 0.0f);
	frag_normal = transpose(inverse(mat3(model_matrix))) * normal;
	gl_Position = MVP * vec4(position, 1.0f);
}