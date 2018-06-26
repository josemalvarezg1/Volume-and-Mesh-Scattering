#version 330
out vec4 color;
in vec2 frag_tex_coords;

uniform sampler2D quad_texture;

void main()
{
	color = texture(quad_texture, frag_tex_coords);
}