#version 330
layout(location = 0) in vec3 vertex_coords;
layout(location = 1) in vec3 volume_coords;
uniform mat4 MVP;

out vec3 in_coord;

void main()
{
	in_coord = volume_coords;
    gl_Position = MVP * vec4(vertex_coords, 1.0);
}