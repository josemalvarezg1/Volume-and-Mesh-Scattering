#version 330 core
layout (location = 0) in vec2 vert_coords;
layout (location = 1) in vec2 tex_coords;

uniform vec2 displacement;

out vec2 texture_c;

void main()
{
	texture_c = tex_coords;
	gl_Position = vec4((vert_coords + displacement), 0.0f, 1.0f);
}