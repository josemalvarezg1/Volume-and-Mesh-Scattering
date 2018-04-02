#version 330 core
out vec4 color;
in vec2 textureC;

uniform sampler2D textureId;

void main()
{             
	color =  texture(textureId, textureC);
}