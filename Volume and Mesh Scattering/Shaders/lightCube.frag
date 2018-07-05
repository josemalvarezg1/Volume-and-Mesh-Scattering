#version 330

layout(location = 0) out vec4 out_color;

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;
uniform float iteration;
uniform vec3 light_pos;
uniform vec3 normal;
uniform mat4 vp_matrix;


in vec3 in_coord;
in vec3 frag_pos;
in float ray_distance;

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
	vec2 texel_size;

	value = texture(volume_text, in_coord).x;
	actual_color = texture(transfer_function_text, value);
	accumulated_color = vec4(0.0f);
	color = vec4(0.0f);

	if (iteration == 0.0)
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
}