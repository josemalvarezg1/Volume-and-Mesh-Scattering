#version 330 core
layout (location = 0) in vec2 vertCoords;
layout (location = 1) in vec4 color;

out vec4 finalColor;

void main()
{
	finalColor = color;
	gl_Position =  vec4((vertCoords), 0.0f, 1.0f);
}