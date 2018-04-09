#version 330
out vec4 color;
in vec2 frag_tex_coords;

uniform sampler2DArray position_tex;

void main() 
{
    color = texture(position_tex, vec3(frag_tex_coords, 1));
}