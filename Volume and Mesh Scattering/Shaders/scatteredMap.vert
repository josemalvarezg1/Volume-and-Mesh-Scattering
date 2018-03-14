#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model_matrix;
uniform mat4 camera_matrix;

void main()
{
	gl_Position = camera_matrix *  model_matrix * vec4(position, 1.0f);
	FragPos = vec3(model_matrix * vec4(position, 1.0f));

	mat3 normalMatrix = transpose(inverse(mat3(model_matrix)));
	Normal = normalMatrix * normal;
}