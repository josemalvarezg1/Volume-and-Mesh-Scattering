#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat4 model_matrix;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 color_frag;

void main()
{
	if(normal == vec3(1.0f, 0.0f, 0.0f))
		color_frag = vec4(1.0f, 0.25f, 0.0f, 1.0f);
	else if(normal == vec3(-1.0f, 0.0f, 0.0f))
		color_frag = vec4(0.196078f, 0.8f, 0.196078f, 1.0f);
	else
		color_frag = vec4(0.958824f, 0.958824f, 0.639216f, 1.0f);
	frag_pos = vec3(model_matrix * vec4(position, 1.0));
	frag_normal = mat3(transpose(inverse(model_matrix))) * normal;
	gl_Position = MVP * vec4(position, 1.0f);
}