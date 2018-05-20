#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2DArray scattered_map;
uniform sampler2DArray depth_map;
uniform vec3 camera_pos;
uniform float epsilon;
uniform float refractive_index;
uniform int n_cameras;
uniform mat4 cameras_matrix[16];
uniform vec3 cameras_dirs[16];
uniform float gamma;
uniform int current_frame;
uniform int g_width;
uniform int g_height;

out vec4 color;

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
	vec3 xo, no, wo, pos, offset, dir;
	float div, vi, visibility, bias, fresnel;
	vec4 texture_pos, sample_color_map;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - xo);
	fresnel = fresnel_t(wo, no, 1.0f / refractive_index);
	
	div = 0.0f;
	color = vec4(0.0f);

	for (int i = 0; i < n_cameras; i++)
	{
		dir = cameras_dirs[i];
		offset = epsilon * (no - dir * dot(no, dir));
		pos = xo - offset;
		texture_pos = cameras_matrix[i] * vec4(pos, 1.0f);
		texture_pos.xyz /= texture_pos.w;
		texture_pos = texture_pos * 0.5 + 0.5;
		
		visibility = 1.0f;
		bias = 0.005 * tan(acos(dot(no, dir)));
		bias = clamp(bias, 0.01f, 0.02f);

		if (texture(depth_map, vec3(texture_pos.xy, i)).z  <  texture_pos.z - bias)
			visibility = 0.0f;

		sample_color_map = sample_color_map(vec3(texture_pos.xy, i));
		color += (sample_color_map / max(sample_color_map.a, 1.0f)) * visibility;
		div += visibility;
	}

	color /= max(div, 1.0);
	color *= clamp(fresnel, 0.0f, 1.0f);
	color = pow(vec4(1) - exp(-color), vec4(1.0/gamma));
	color = pow(color, vec4(1.0f / gamma));
}