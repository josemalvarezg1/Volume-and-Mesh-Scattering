#version 420

layout(location = 0) in vec2 vertex_coords;
layout(location = 1) in vec2 volume_coords;

uniform mat4 MVP;
uniform vec3 position;
uniform int axis;
uniform float start_texture;
uniform mat4 model_matrix;

out vec3 in_coord;
out vec3 frag_pos;
out float ray_distance;

void main()
{
	vec3 new_position, new_texture;
	float new_distance;

	if (axis == 0)
	{
		new_position = vec3(position.x, vertex_coords.x, vertex_coords.y);
		new_texture = vec3(start_texture, volume_coords.x, volume_coords.y);
		new_distance = abs(position.x);
	}
	else if (axis == 1)
	{
		new_position = vec3(vertex_coords.x, position.y, vertex_coords.y);
		new_texture = vec3(volume_coords.x, start_texture, volume_coords.y);
		new_distance = abs(position.y);
	}
	else if (axis == 2)
	{
		new_position = vec3(vertex_coords.x, vertex_coords.y, position.z);
		new_texture = vec3(volume_coords.x, volume_coords.y, start_texture);
		new_distance = abs(position.z);
	}

	ray_distance = new_distance;
	in_coord = new_texture;
	frag_pos = vec3(model_matrix * vec4(new_position, 1.0));
    gl_Position = MVP * vec4(new_position, 1.0);
}