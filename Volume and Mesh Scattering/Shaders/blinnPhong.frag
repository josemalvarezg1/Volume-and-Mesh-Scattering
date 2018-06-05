#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 diffuse_reflectance;
uniform vec3 light_ambient_color;
uniform vec3 light_diffuse_color;
uniform vec3 light_specular_color;
uniform float gamma;

out vec4 color;

void main(void)
{
	vec3 normal, view_dir, light_dir, halfway_dir;
	vec4 ambient, diffuse, specular;
	float diff, spec;

	ambient = vec4(0.0f);
	normal = normalize(frag_normal);

	light_dir = normalize(light_pos - frag_pos);
	diff = max(dot(normal, light_dir), 0.0f);
	diffuse = vec4(diff, diff, diff, 1.0f);

	view_dir = normalize(view_pos - frag_pos);
	halfway_dir = normalize(light_dir + view_dir);
	spec = pow(max(dot(normal, halfway_dir), 0.0f), 128.0f);
	specular = vec4(spec, spec, spec, 1.0f);

	color = (ambient * vec4(light_ambient_color, 1.0f) + vec4(diffuse_reflectance, 1.0) * diffuse * vec4(light_diffuse_color, 1.0f) + specular * vec4(light_specular_color, 1.0f));
	color = pow(color, vec4(1.0f / gamma));
}