#version 330 core
out vec4 color;
in vec4 final_color;

void main()
{             
	color = final_color;
}