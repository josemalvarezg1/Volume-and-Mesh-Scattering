#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2DArray scattered_map;
uniform sampler2DArrayShadow depth_map;
uniform vec3 camera_pos;
uniform vec4 light_pos;
uniform float bias;
uniform float epsilon;
uniform float refractive_index;
uniform int n_cameras;
uniform mat4 cameras_matrix[1];
uniform vec3 cameras_dirs[1];
uniform float gamma;
uniform int current_frame;

out vec4 color;

#define ARRAY_TEX_STEP 1.0f / 1024.0f
#define KERNEL_SIZE 5

const vec2 kernel[KERNEL_SIZE] = {
	vec2(0),
	0.5 * ARRAY_TEX_STEP * vec2(-1.0f, 3.0f),
	0.5 * ARRAY_TEX_STEP * vec2(3.0f, 1.0f),
	0.5 * ARRAY_TEX_STEP * vec2(1.0f, -3.0f),
	0.5 * ARRAY_TEX_STEP * vec2(-3.0f, 1.0f)
};

float sample_shadow_map(vec4 light_pos)
{
	light_pos.w -= bias;
	if (light_pos.x < 0.0 || light_pos.x > 1.0) return 1.0;
	if (light_pos.y < 0.0 || light_pos.y > 1.0) return 1.0;
	return texture(depth_map, light_pos).r;
}

vec4 sample_color_map(vec3 coord)
{
	return texture(scattered_map, coord);
}

float fresnel_t(vec3 inv, vec3 n, float n_1)
{
	float cos_i, n_2, eta, sin_t;
	cos_i = clamp(-1.0f, 1.0f, dot(inv, n));
	n_2 = 1.0f;
	eta = n_1 / n_2;

	sin_t = eta * sqrt(max(0.0f, 1.0f - cos_i * cos_i));
	if (sin_t >= 1.0f)
		return 1.0f;

	float cos_t, const_t, cos_i_2, factor_1, factor_2, Ts, Tp;
	cos_t = sqrt(max(0.0f, 1.0f - sin_t * sin_t));

	const_t = eta * (cos_t / cos_i);
	cos_i_2 = (2 * cos_i);
	factor_1 = cos_i_2 / ((eta * cos_i) + cos_t);
	factor_2 = cos_i_2 / ((eta * cos_t) + cos_i);

	Ts = const_t * (factor_1 * factor_1);
	Tp = const_t * (factor_2 * factor_2);

	return (Ts + Tp) / 2.0f;
}

void main(void)
{
	vec3 no = normalize(frag_normal);
	vec3 wo = normalize(camera_pos - frag_pos);
	color = vec4(0.0f);
	float div = 0.0f;
	vec3 pos, offset, dir;
	for (int i = 0; i < n_cameras; i++)
	{
		dir = cameras_dirs[i];
		offset = epsilon * (no - dir * dot(no, dir));
		pos = frag_pos - offset;
		vec4 l = cameras_matrix[i] * vec4(pos, 1.0f);
		float visibility = 1.0f;
		for (int j = 0; j < KERNEL_SIZE; j++)
		{
			float offset = 1.0f;
			vec4 kernel_j = vec4(l.xy + offset * kernel[j], i, l.z);
			float vi = sample_shadow_map(kernel_j.xyzw);
			visibility *= vi;
		}
		vec4 sample_color_map = sample_color_map(vec3(l.xy, i));
		color += (sample_color_map / max(sample_color_map.a, 1)) * visibility;
		div += visibility;
	}

	color /= max(div, 1.0);

	float fresnel = fresnel_t(wo, no, 1.0f / refractive_index);
	color *= clamp(fresnel, 0.0f, 1.0f);
}