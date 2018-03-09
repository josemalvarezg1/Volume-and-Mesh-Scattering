#version 330

uniform mat4 view_matrix; //Matriz de View
uniform mat4 projection_matrix; //Matriz de proyeccion
uniform mat4 model_matrix; //Matriz de model
uniform vec3 lightPos; //Posicion de luz
uniform vec3 view; //Vector de EYE
uniform float shinyBlinn;

layout (location=0) in vec3 position; //Posicion de los vertices
layout (location=1) in vec3 normal; //Posicion de normales

out vec3 FragPos;
out vec3 normales;
out vec3 viewPos;
out vec3 lightPosfrag;

void main() {

	//Posicion de los vertices
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0); //La posicion viene por la mult de matrices
	mat4 model = model_matrix;

	FragPos = vec3(model * vec4(position, 1.0));
	lightPosfrag = lightPos;
	viewPos = view;
	normales = mat3(transpose(inverse(model))) * normal;

}