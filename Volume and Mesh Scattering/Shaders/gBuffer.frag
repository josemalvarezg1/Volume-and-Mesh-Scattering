#version 330
layout (location = 0) out vec3 g_out;

in vec3 frag_pos;
in vec3 frag_normal;
uniform int num_of_buffer;

void main()
{    
	if (num_of_buffer == 0)
		g_out = frag_pos;
	else
		g_out = normalize(frag_normal);
}