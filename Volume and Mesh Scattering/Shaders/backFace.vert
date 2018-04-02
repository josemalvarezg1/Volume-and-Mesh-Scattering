#version 330
layout(location = 0) in vec3 vertexCoords;
layout(location = 1) in vec3 volumeCoords;
uniform mat4 MVP;

out vec3 coordinate;

void main()
{
	coordinate = volumeCoords;
    gl_Position = MVP * vec4(vertexCoords, 1.0);
}
