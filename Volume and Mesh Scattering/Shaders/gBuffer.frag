#version 330
layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;

in vec3 frag_pos;
in vec3 frag_normal;

void main()
{    
    g_position = frag_pos;
    g_normal = normalize(frag_normal);
}