#version 330 core
layout (location = 0) in vec2 vertCoords;
layout (location = 1) in vec2 texCoords;

uniform vec2 displacement;

out vec2 textureC;

void main()
{
	textureC = texCoords;
	gl_Position = vec4((vertCoords + displacement), 0.0f, 1.0f);
}