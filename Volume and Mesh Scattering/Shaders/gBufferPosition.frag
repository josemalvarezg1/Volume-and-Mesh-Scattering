#version 330
out vec4 color;
in vec2 frag_tex_coords;

uniform sampler2DArray quad_texture;
uniform int camera_select;

void main()
{
	color = texture(quad_texture, vec3(frag_tex_coords, camera_select));
}