#version 330
layout(location = 0) in vec3 vertex_coords;
layout(location = 1) in vec3 volume_coords;
uniform mat4 MVP;
uniform mat4 model;

out vec3 in_coord;
out vec3 frag_pos;

void main()
{
	in_coord = volume_coords;
	frag_pos = vec3(model * vec4(vertex_coords, 1.0));
    gl_Position = MVP * vec4(vertex_coords, 1.0);
}
