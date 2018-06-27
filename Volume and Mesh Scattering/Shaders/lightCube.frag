#version 420

layout(location = 0) out vec4 out_color;

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;
uniform float iteration;
uniform int actual_texture;
uniform vec3 light_pos;
uniform vec3 normal;
uniform mat4 vp_matrix;
uniform ivec3 volume_size;
uniform float alpha_0;
uniform float alpha_1;
uniform int direction;

in vec3 in_coord;
in vec3 frag_pos;
in float ray_distance;

uniform layout(binding = 4, rgba16f) restrict image3D vol_ilum;

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
	vec4 accumulated_color, actual_color, color, offset, I_0, I_1, S;
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
		ray_direction = normalize(light_pos - frag_pos);
		intersection_point = plane_intersection(frag_pos, ray_direction);
		offset = vec4(intersection_point, 1.0f);
		offset = vp_matrix * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
		accumulated_color = texture(previous_text, offset.xy);
		//Aplicar Kernel
		color.rgb = (1.0f - actual_color.a) * accumulated_color.rgb + actual_color.a * actual_color.rgb;
		color.a = (1.0f - actual_color.a) * accumulated_color.a + actual_color.a;
	}

	out_color = color;

	size = ivec3(in_coord.x * volume_size.x, in_coord.y * volume_size.y, in_coord.z * volume_size.z);

	if (direction == 0)
		imageStore(vol_ilum, size, color);
	else
	{
		I_0 = imageLoad(vol_ilum, size);
		I_1 = color;
		S = alpha_0 * I_0 + alpha_1 * I_1;
		imageStore(vol_ilum, size, S);
	}
	
}