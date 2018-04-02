#version 330
layout(location = 0) in vec3 vertexCoords;
layout(location = 1) in vec3 volumeCoords;
uniform mat4 MVP;
uniform mat4 model;

out vec3 inCoordinate;
out vec3 positionFrag;

void main()
{
	inCoordinate = volumeCoords;
	positionFrag = vec3(model * vec4(vertexCoords, 1.0));
    gl_Position = MVP * vec4(vertexCoords, 1.0);
}
