#version 330

in vec3 frag_normal; //Normales
in vec3 light_pos_frag; //Posicion de la luz
in vec3 frag_pos; //Posicion del fragmento
in vec3 eye_pos; //Posicion de la vista

out vec4 color; //Color final

void main() 
{

	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 norm = normalize(frag_normal);
	vec3 lightDir = normalize(light_pos_frag - frag_pos);
			
	vec4 ambient = vec4(0.0,0.0,0.0,1.0);

	//Lambert	
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = vec4(diff,diff,diff,1.0);

	//Blinn-Phong
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 viewDir = normalize(eye_pos - frag_pos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
	specular = vec4(spec,spec,spec,1.0);
    
	//Resultado final
	result = (ambient + diffuse + specular);
	color =  result;

}