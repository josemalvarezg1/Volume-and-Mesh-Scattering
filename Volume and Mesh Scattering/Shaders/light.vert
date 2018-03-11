#version 330
layout(location = 0) in vec3 vertexCoords;
uniform mat4 MVP;
void main()
{
	gl_Position = MVP * vec4(vertexCoords, 1.0f);
}