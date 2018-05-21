#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat4 model_matrix;
uniform int index;

out vec3 frag_pos;
out vec3 frag_normal;

void main()
{
	float normal_sign;
	if (index == 1 || index == 3)
		normal_sign = -1.0f;
	else
		normal_sign = 1.0f;
	frag_pos = vec3(model_matrix * vec4(position, 1.0));
	frag_normal = mat3(transpose(inverse(model_matrix))) * normal * normal_sign;
	gl_Position = MVP * vec4(position, 1.0f);
}