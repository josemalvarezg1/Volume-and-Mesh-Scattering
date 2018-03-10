#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    vec4 worldPos = model_matrix * vec4(position, 1.0f);
    FragPos = worldPos.xyz; 
    gl_Position = projection_matrix * view_matrix * worldPos;
    
    mat3 normalMatrix = transpose(inverse(mat3(model_matrix)));
    Normal = normalMatrix * normal;
}