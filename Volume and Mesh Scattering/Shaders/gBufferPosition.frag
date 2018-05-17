#version 330
out vec4 color;
in vec2 frag_tex_coords;

uniform sampler2D position_tex;

void main() 
{
    color = texture(position_tex, frag_tex_coords);
}