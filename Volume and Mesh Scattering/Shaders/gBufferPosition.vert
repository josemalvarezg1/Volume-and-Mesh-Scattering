#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 model_matrix;
out vec2 TexCoords;

void main()
{
    gl_Position = model_matrix  * vec4(position, 1.0f);
    TexCoords = texCoords;
}