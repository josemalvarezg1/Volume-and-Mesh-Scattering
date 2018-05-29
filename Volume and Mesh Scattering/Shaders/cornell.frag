#version 330

uniform int index;
uniform vec3 light_pos;

in vec3 frag_normal;
in vec3 frag_pos;
in vec4 color_frag;

out vec4 color;

void main()
{
	vec3 light_dir, normal;
	float diffuse;

	color = color_frag;
	light_dir = normalize(light_pos - frag_pos);
	normal = normalize(frag_normal);
	diffuse = max(dot(normal, light_dir), 0.0f);
	color *= vec4(diffuse, diffuse, diffuse, 1.0f);
}
   