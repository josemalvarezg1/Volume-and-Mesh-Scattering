#version 330

layout (location = 0) out vec4 color_out_1;
layout (location = 1) out vec4 color_out_2;

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;
uniform float iteration;
uniform int actual_texture;
uniform vec3 light_pos;
uniform vec3 normal;
uniform vec3 ray_distance;
//uniform layout(binding=10, rgba16f) writeonly image2D vol_ilum;

in vec3 in_coord;
in vec3 frag_pos;


vec3 plane_intersection(vec3 origin, vec3 direction) 
{
	vec3 t;
	t = -(normal.x * origin.x + normal.y * origin.y + normal.z * origin.z + ray_distance) / (normal.x * direction.x + normal.y * direction.y + normal.z * direction.z);
	return normal.x * (origin.x + direction.x * t) + normal.y * (origin.y + direction.y * t) + normal.z * (origin.z + direction.z * t) + ray_distance;
}

void main() 
{	
	float value;
	vec3 ray_direction, intersection_point;
	vec4 accumulated_color, actual_color, color;

	value = texture(volume_text, in_coord).r;
	actual_color = texture(transfer_function_text, value);
	
	if (iteration == 0.0f)
	{
		color.rgb = (1.0f - actual_color.a) + actual_color.a * actual_color.rgb;
		color.a = (1.0f - actual_color.a) + actual_color.a;
	}
	else
	{
		// Calcular intersection_point
		ray_direction = normalize(light_pos - frag_pos);
		intersection_point = plane_intersection(frag_pos, ray_direction);
		accumulated_color = texture(previous_text, vec2(intersection_point));
		color.rgb = (1.0f - actual_color.a) * accumulated_color.rgb + actual_color.a * actual_color.rgb;
		color.a = (1.0f - actual_color.a) * accumulated_color.a + actual_color.a;
	}

	if(actual_texture == 0)
		color_out_1 = color;
	else
		color_out_2 = color;

	//imageStore(vol_ilum, ,color);
}