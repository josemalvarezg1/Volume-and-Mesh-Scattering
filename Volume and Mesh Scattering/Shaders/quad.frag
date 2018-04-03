#version 330 core
out vec4 color;
in vec2 texture_c;

uniform sampler2D texture_id;

void main()
{             
	color =  texture(texture_id, texture_c);
}