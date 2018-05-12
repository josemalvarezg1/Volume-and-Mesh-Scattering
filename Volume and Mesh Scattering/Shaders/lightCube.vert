#version 330
layout(location = 0) in vec2 vertex_coords;
layout(location = 1) in vec2 volume_coords;
uniform mat4 MVP;

out vec2 in_coord;

void main()
{
	float pos_x, pos_y, pos_z;
	in_coord = volume_coords;

	//vertex_coords;

    gl_Position = MVP * vec4(pos_x, pos_y, pos_z, 1.0);
}