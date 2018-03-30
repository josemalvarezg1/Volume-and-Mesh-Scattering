#version 330
out vec4 color;
in vec2 TexCoords;

uniform sampler2DArray position_tex;

void main() 
{
    color = texture(position_tex, vec3(TexCoords, 0));
}