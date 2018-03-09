#version 330

uniform vec3 view; //Vector de EYE
uniform float shinyBlinn;

in vec3 normales; //Normales
in vec3 lightPosfrag; //Posicion de la luz
in vec3 FragPos; //Posicion del fragmento
in vec3 viewPos; //Posicion de la vista

out vec4 color; //Color final

void main() {

	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 norm = normalize(normales);
	vec3 lightDir = normalize(lightPosfrag - FragPos);
			
	vec4 ambient = vec4(0.0,0.0,0.0,1.0);

	//Lambert	
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = vec4(diff,diff,diff,1.0);

	//Blinn-Phong
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), shinyBlinn);
	specular = vec4(spec,spec,spec,1.0);
    
	//Resultado final
	result = (ambient + diffuse + specular);
	color =  vec4(norm, 1.0f);

}