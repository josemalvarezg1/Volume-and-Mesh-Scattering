#version 420

layout (location = 0) out vec4 color_1;
layout (location = 1) out vec4 color_2;

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;
uniform float iteration;
uniform int actual_texture;
uniform vec3 light_pos;
uniform vec3 normal;
uniform mat4 vp_matrix;
uniform ivec3 volume_size;

in vec3 in_coord;
in vec3 frag_pos;
in float ray_distance;

//uniform layout(binding=4, rgba16f) writeonly image3D vol_ilum;

vec3 plane_intersection(vec3 origin, vec3 direction) 
{
	float t;
	t = -(normal.x * origin.x + normal.y * origin.y + normal.z * origin.z + ray_distance) / (normal.x * direction.x + normal.y * direction.y + normal.z * direction.z);
	return (origin + direction * t);
}

void main() 
{
	float value;
	vec3 ray_direction, intersection_point;
	vec4 accumulated_color, actual_color, color, offset;
	ivec3 size;

	value = texture(volume_text, in_coord).x;
	actual_color = texture(transfer_function_text, value);
	
	if (iteration == 0.0f)
	{
		color.rgb = (1.0f - actual_color.a) + actual_color.a * actual_color.rgb;
		color.a = (1.0f - actual_color.a) + actual_color.a;
	}
	else
	{
		//ray_direction = normalize(light_pos - frag_pos);
		//intersection_point = plane_intersection(frag_pos, ray_direction);
		//offset = vec4(intersection_point, 1.0f);
		//offset = vp_matrix * offset;
		//offset.xyz /= offset.w;   
		//offset.xyz = offset.xyz * 0.5 + 0.5;
		//accumulated_color = texture(previous_text, offset.xy);
		//color.rgb = (1.0f - actual_color.a) * accumulated_color.rgb + actual_color.a * actual_color.rgb;
		//color.a = (1.0f - actual_color.a) * accumulated_color.a + actual_color.a;
		color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (actual_texture == 0)
		color_1 = color;
	else
		color_2 = color;

	//size = ivec3(in_coord.x * volume_size.x, in_coord.y * volume_size.y, in_coord.z * volume_size.z);

	//imageStore(vol_ilum, size, color);
}