#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 frag_pos;
out vec3 frag_normal;

uniform mat4 model_matrix;
uniform mat4 camera_matrix;

void main()
{
	gl_Position = camera_matrix *  model_matrix * vec4(position, 1.0f);
	frag_pos = vec3(model_matrix * vec4(position, 1.0f));

	mat3 normalMatrix = transpose(inverse(mat3(model_matrix)));
	frag_normal = normalMatrix * normal;
}