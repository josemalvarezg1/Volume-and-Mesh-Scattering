#version 330
uniform vec3 diffuse_color;
out vec4 color;

void main()
{
	color = vec4(diffuse_color, 1.0f);
}