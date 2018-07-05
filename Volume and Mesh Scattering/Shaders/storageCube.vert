#version 430

layout(location = 0) in vec2 vertex_coords;
layout(location = 1) in vec2 volume_coords;

uniform vec3 position;
uniform int axis;
uniform float start_texture;
uniform mat4 model_matrix;

out vec3 in_coord;
out vec3 frag_pos;

void main()
{
	vec3 new_position, new_texture, new_frag_pos;

	if (axis == 0)
	{
		new_position = vec3(position.x, vertex_coords.x * 2.0f, vertex_coords.y * 2.0f);
		new_frag_pos = vec3(position.x, vertex_coords.x, vertex_coords.y);
		new_texture = vec3(start_texture, volume_coords.x, volume_coords.y);
	}
	else if (axis == 1)
	{
		new_position = vec3(vertex_coords.x * 2.0f, position.y, vertex_coords.y * 2.0f);
		new_frag_pos = vec3(vertex_coords.x, position.y, vertex_coords.y);
		new_texture = vec3(volume_coords.x, start_texture, volume_coords.y);
	}
	else if (axis == 2)
	{
		new_position = vec3(vertex_coords.x * 2.0f, vertex_coords.y * 2.0f, position.z);
		new_frag_pos = vec3(vertex_coords.x, vertex_coords.y, position.z);
		new_texture = vec3(volume_coords.x, volume_coords.y, start_texture);
	}

	in_coord = new_texture;
	frag_pos = vec3(model_matrix * vec4(new_frag_pos, 1.0));
    gl_Position = vec4(new_position, 1.0);
}