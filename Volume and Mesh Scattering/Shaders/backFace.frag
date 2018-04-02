#version 330
in vec3 coordinate;
out vec4 color;

void main()
{
    color = vec4(coordinate, 1.0);
}