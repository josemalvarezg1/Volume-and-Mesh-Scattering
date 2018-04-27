#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2DArray scattered_map;
uniform sampler2DArray depth_map;
uniform vec3 camera_pos;
uniform float epsilon;
uniform float refractive_index;
uniform int n_cameras;
uniform mat4 cameras_matrix[6];
uniform vec3 cameras_dirs[6];
uniform float gamma;
uniform int current_frame;
uniform int g_width;
uniform int g_height;

vec2 poissonDisk[4] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760)
);

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
	vec3 xo = frag_pos;
	vec3 no = normalize(frag_normal);
	vec3 wd;
	vec3 wo = normalize(camera_pos - xo);
	vec3 xo_shrink = xo - gamma * (no - wo * dot(no, wo));
	color = vec4(0.0f);
	float div = 0.0f, vi;
	vec3 pos, offset, dir;
	vec4 l;
	for (int i = 0; i < n_cameras; i++)
	{
		dir = cameras_dirs[i];
		offset = epsilon * (no - dir * dot(no, dir));
		pos = xo - offset;
		l = cameras_matrix[i] * vec4(pos, 1.0f);
		l.xyz /= l.w;
		l = l * 0.5 + 0.5;
		float visibility = 1.0f;
		float bias = 0.005 * tan(acos(dot(no, dir)));
		bias = clamp(bias, 0.01f, 0.02f);
		if (texture(depth_map, vec3(l.xy, i)).z  <  l.z - bias)
		{
			for (int k = 0; k < 4; k++) {
				if (texture(depth_map, vec3(l.xy + poissonDisk[k] / 700.0f, l)).z  <  l.z - bias) {
					visibility -= 0.2f;
				}
			}
		}
		vec4 sample_color_map = sample_color_map(vec3(l.xy, i));
		color += (sample_color_map / max(sample_color_map.a, 1.0f)) * visibility;
		div += visibility;
	}
	color /= max(div, 1.0);
	float fresnel = fresnel_t(wo, no, 1.0f / refractive_index);
	color *= clamp(fresnel, 0.0f, 1.0f);
	// Corrección Gamma
	color = pow(color, vec4(1.0f / gamma));
}