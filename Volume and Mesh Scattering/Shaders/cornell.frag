#version 330

uniform int index;
uniform vec3 light_pos;

in vec3 frag_normal;
in vec3 frag_pos;

out vec4 color;

void main()
{
	vec3 light_dir, normal;
	float diffuse;

	if (index == 0)
		color = vec4(0.25f, 0.25f, 1.0f, 1.0f);
	else if (index == 1 || index == 4 || index == 5)
		color = vec4(0.958824f, 0.958824f, 0.639216f, 1.0f);
	else if (index == 2)
		color = vec4(0.196078f, 0.8f, 0.196078f, 1.0f);
	else if (index == 3)
		color = vec4(1.0f, 0.25f, 0.0f, 1.0f);

	light_dir = normalize(light_pos - frag_pos);
	normal = normalize(frag_normal);
	diffuse = max(dot(normal, light_dir), 0.0);
	color *= vec4(diffuse, diffuse, diffuse, 1.0);
}
   