#version 330 core
layout (location = 0) in vec2 vert_coords;
layout (location = 1) in vec4 color;

out vec4 final_color;

void main()
{
	final_color = color;
	gl_Position =  vec4((vert_coords), 0.0f, 1.0f);
}