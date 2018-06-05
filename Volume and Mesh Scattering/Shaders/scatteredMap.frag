#version 330
in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 camera_pos;

uniform vec3 light_pos;
uniform vec3 light_diffuse_color;

uniform float asymmetry_param_g;
uniform float refractive_index;
uniform vec3 diffuse_reflectance;
uniform mat4 vp_light;

uniform int n_samples;
uniform vec2 samples[96];
uniform mat4 model_matrix;
uniform sampler2DArray g_position;
uniform sampler2DArray g_normal;
uniform sampler2DArray g_depth;
uniform float radius;
uniform float bias;
uniform vec3 model_center;

// Valores pre-calculados
uniform vec3 attenuation_coeff;
uniform vec3 D;
uniform vec3 effective_transport_coeff;
uniform float c_phi_1;
uniform float c_phi_2;
uniform float c_e;
uniform float A;
uniform vec3 de;
uniform vec3 zr;

const float PI = 3.1415926535897932384626433832795f;
out vec4 color;

vec3 calculate_ni_ast(vec3 xo, vec3 xi, vec3 ni)
{
	if (xo == xi)
		return ni;
	else
	{
		vec3 factor_1, factor_2;
		factor_1 = normalize(xo - xi);
		factor_2 = normalize(cross(ni, xo - xi));
		return cross(factor_1, factor_2);
	}
}

vec3 calculate_dr_pow(vec3 r, vec3 D, float miu_0, vec3 de, vec3 cos_beta, vec3 attenuation_coeff)
{
	if (miu_0 > 0.0f)
		return pow(r, vec3(2.0f)) + D * miu_0 * (D * miu_0 - 2.0f * de * cos_beta);
	else
		return pow(r, vec3(2.0f)) + 1.0f / (pow(3.0f * attenuation_coeff, vec3(2.0f)));
}

vec3 diffuse_part_prime(vec3 x, vec3 w12, vec3 r, vec3 no)
{
	vec3 effec_r, one_effec_r, factor_1, factor_2, factor_3, factor_4;
	float x_dot_w12, w12_dot_no, x_dot_no;
	effec_r = effective_transport_coeff * r;
	one_effec_r = vec3(1.0f) + effec_r;
	x_dot_w12 = dot(x, w12);
	w12_dot_no = dot(w12, no);
	x_dot_no = dot(x, no);
	factor_1 = (1.0f / (4.0f * c_phi_2)) * 0.02533f * (exp(-effec_r) / pow(r, vec3(3.0f)));
	factor_2 = c_phi_1 * ((r * r) / D + 3.0f * one_effec_r * x_dot_w12);
	factor_3 = 3.0f * D * one_effec_r * w12_dot_no;
	factor_4 = (one_effec_r + 3.0f * D * (3.0f * one_effec_r + effec_r * effec_r) / (r * r) * x_dot_w12) * x_dot_no;
	return factor_1 * (factor_2 - c_e * (factor_3 - factor_4));
}

float fresnel_t(vec3 inv, vec3 n, float n_1)
{
	float cos_i, n_2, eta, sin_t;
	cos_i = clamp(dot(inv, n), -1.0f, 1.0f);
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

void main()
{
	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, dr, dr_pow, w12, p, ni_ast, xv, dv, wv;
	vec3 diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
	vec3 cos_beta, z_prime, R, T, diffuse_part;
	float miu_0, Ti, To, theta, visibility;
	mat2 rotation_samples_matrix;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - frag_pos);

	Lo = vec3(0.0f);
	Ll = light_diffuse_color;

	wi = normalize(light_pos - model_center);
	/* Inicio: Generación de muestras */
	for (int i = 0; i < n_samples; i++)
	{
		vec4 offset = vec4(frag_pos, 1.0f);
		// De espacio de vista a espacio de clipping
		offset = vp_light * offset;
		offset.xyz /= offset.w;
		// El offset estará en un rango [0:1]     
		offset.xyz = offset.xyz * 0.5f + 0.5f;
		offset.xy += samples[i].xy;
		theta = 2.0f * PI * radius;
		rotation_samples_matrix = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
		offset.xy = rotation_samples_matrix * offset.xy;

		xi = texture(g_position, vec3(offset.xy, 0)).xyz;
		ni = texture(g_normal, vec3(offset.xy, 0)).xyz;

		visibility = 1.0f;

		if (texture(g_depth, vec3(offset.xy, 0)).r < offset.z - bias)
			visibility = 0.0f;

		if (visibility > 0.0f)
		{
			x = xo - xi;
			r = vec3(length(x));
			w12 = refract(wi, ni, 1.0f / refractive_index);

			/* Inicio: Parte Difusa */

			ni_ast = calculate_ni_ast(xo, xi, ni);

			xv = xi + (2.0f * A * de * ni_ast);
			dv = vec3(length(xo - xv));
			wv = w12 - (2.0f * (dot(w12, ni_ast)) * ni_ast);

			cos_beta = -sqrt((pow(r, vec3(2.0f)) - pow(dot(x, w12), 2)) / (pow(r, vec3(2.0f)) + pow(de, vec3(2.0f))));
			miu_0 = dot(-no, w12);
			dr_pow = calculate_dr_pow(r, D, miu_0, de, cos_beta, attenuation_coeff);
			dr = sqrt(dr_pow);

			diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, no);
			diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, no);
			diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;

			Ti = fresnel_t(wi, ni, 1.0f / refractive_index);
			To = fresnel_t(wo, no, 1.0f / refractive_index);

			diffuse_part = (Ti * diffuse_part_d * dot(ni, wi));
			//diffuse_part = (diffuse_part_d * dot(ni, wi));

			Lo += diffuse_part;

			/* Fin: Parte Difusa */
		}
	}

	Lo *= ((PI * Ll) / n_samples);

	/* Fin: Generación de muestras */

	color = vec4(Lo * diffuse_reflectance, 1.0f);
}