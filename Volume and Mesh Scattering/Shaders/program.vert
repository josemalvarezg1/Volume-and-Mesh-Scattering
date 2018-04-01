#version 330

layout (location=0) in vec3 position; //Posicion de los vertices
layout (location=1) in vec3 normal; //Posicion de normales

uniform mat4 view_matrix; //Matriz de View
uniform mat4 projection_matrix; //Matriz de proyeccion
uniform mat4 model_matrix; //Matriz de model
uniform vec3 light_pos; //Posicion de luz
uniform vec3 eye; //Vector de EYE

out vec3 frag_pos;
out vec3 frag_normal;
out vec3 eye_pos;
out vec3 light_pos_frag;

void main() {

	//Posicion de los vertices
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0); //La posicion viene por la mult de matrices
	mat4 model = model_matrix;

	frag_pos = vec3(model * vec4(position, 1.0));
	light_pos_frag = light_pos;
	eye_pos = eye;
	frag_normal = mat3(transpose(inverse(model))) * normal;
}