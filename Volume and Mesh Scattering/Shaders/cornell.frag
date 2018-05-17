#version 330

in vec3 frag_pos;
in vec3 frag_normal;
in vec3 frag_light_pos;

out vec4 color;

void main()
{
	//Refleccion Difusa
	vec3 norm = normalize(frag_normal);
	vec3 light_dir = normalize(frag_light_pos - frag_pos);

	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);

	float diff = max(dot(norm, light_dir), 0.0);
	diffuse = vec4(diff, diff, diff, 1.0);

	//Blinn-Phong
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 viewDir = normalize(vec3(0.0, 0.0, 3.0) - frag_pos);
	vec3 halfwayDir = normalize(light_dir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0f);
	specular = vec4(spec, spec, spec, 1.0);

	color = vec4(1.0f, 0.0f, 1.0f, 1.0f) + diffuse + specular;
}