#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2DArray scattered_map;
uniform sampler2DArray depth_map;
uniform vec3 camera_pos;
uniform float epsilon;
uniform float refractive_index;
uniform int n_cameras;
uniform mat4 cameras_matrix[32];
uniform vec3 cameras_dirs[32];
uniform float gamma;
uniform float bias;
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
	cos_i = clamp(dot(inv, n), 0.0f, 1.0f);
	n_2 = 1.0f;
	eta = n_1 / n_2;

	sin_t = eta * sqrt(max(0.0f, 1.0f - cos_i * cos_i));
	if (sin_t >= 1.0f)
		return 1.0f;

	float cos_t, const_t, cos_i_2, factor_1, factor_2, Ts, Tp;
	cos_t = sqrt(max(0.0f, 1.0f - sin_t * sin_t));

	const_t = eta * (cos_t / cos_i);
	cos_i_2 = (2.0f * cos_i);
	factor_1 = cos_i_2 / ((eta * cos_i) + cos_t);
	factor_2 = cos_i_2 / ((eta * cos_t) + cos_i);

	Ts = const_t * (factor_1 * factor_1);
	Tp = const_t * (factor_2 * factor_2);

	return (Ts + Tp) / 2.0f;
}

void main(void)
{
	vec3 xo, no, wo, pos, offset, dir;
	float div, vi, visibility, fresnel, cos_theta, pcf_depth;
	vec4 texture_pos, color_map;
	vec2 texel_size;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - xo);
	fresnel = fresnel_t(wo, no, 1.0f / refractive_index);

	div = 0.0f;
	color = vec4(0.0f);

	for (int i = 0; i < n_cameras; i++)
	{
		dir = normalize(cameras_dirs[i] - xo);
		cos_theta = clamp(dot(no, dir), 0.0f, 1.0f);
		offset = epsilon * (no - dir * cos_theta);
		pos = xo - offset;
		texture_pos = cameras_matrix[i] * vec4(pos, 1.0f);
		texture_pos.xyz /= texture_pos.w;
		texture_pos = texture_pos * 0.5f + 0.5f;

		visibility = 1.0f;

		for (int k = -1; k <= 1; k++) {
			for (int j = -1; j <= 1; j++) {
				texel_size = 1.0f / vec2(g_width, g_height);
				pcf_depth = texture(depth_map, vec3(texture_pos.xy + vec2(k, j) * texel_size, i)).r;
				if (pcf_depth < texture_pos.z - bias)
					visibility -= 1.0f / 9.0f;
			}
		}

		/*if (texture(depth_map, vec3(texture_pos.xy, i)).z  <  texture_pos.z - bias)
			visibility = 0.0f;*/

		color_map = sample_color_map(vec3(texture_pos.xy, i));
		color += (color_map / max(color_map.a, 1.0f)) * visibility;
		div += visibility;
	}

	color /= max(div, 1.0f);
	color *= clamp(fresnel, 0.0f, 1.0f);
	color = pow(color, vec4(1.0f / gamma));
}