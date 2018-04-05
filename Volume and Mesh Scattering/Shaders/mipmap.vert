#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 coord_text;

out vec3 geom_coord_text;

void main()
{
	geom_coord_text = coord_text;
    gl_Position = vec4(position.xy, 0.0f, 1.0f);
}