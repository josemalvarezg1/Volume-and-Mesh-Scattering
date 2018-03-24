#version 330
in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 camera_pos;

uniform vec3 light_pos;
uniform vec4 light_diff;
uniform vec4 light_spec;
uniform vec4 light_amb;

uniform float asymmetry_param_g;
uniform float refractive_index;
uniform vec3 diffuse_reflectance;
uniform mat4 projection_matrix;

uniform int n_samples;
uniform vec2 samples[64];
uniform mat4 model_matrix;
uniform sampler2D g_position;
uniform sampler2D g_normal;

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

const float PI = 3.1415926535897932384626433832795;

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
	if(miu_0 > 0.0f)
		return pow(r, vec3(2.0f)) + D * miu_0 * (D * miu_0 - 2 * de * cos_beta);
	else
		return pow(r, vec3(2.0f)) + 1 / (pow(3 * attenuation_coeff, vec3(2.0f)));
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
	factor_1 = (1 / (4 * c_phi_2)) * 2.4674011 * (exp(-effec_r) / pow(r, vec3(3.0f)));
	factor_2 = c_phi_1 * ((r * r) / D + 3 * one_effec_r * x_dot_w12);
	factor_3 = 3 * D * one_effec_r * w12_dot_no;
	factor_4 = (one_effec_r + 3 * D * (3 * one_effec_r + effec_r * effec_r) / (r * r) * x_dot_w12) * x_dot_no;
	return factor_1 * (factor_2 - c_e * (factor_3 - factor_4));
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

void main() 
{
	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, dr, dr_pow, w12, p, ni_ast, xv, dv, wv;
	vec3 diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
	vec3 cos_beta, z_prime, R, T, diffuse_part;
	float miu_0, Ti, To;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - frag_pos);

	Lo = vec3(0.0f);
	Ll = light_diff.xyz; 		/*Hasta ahora una sola luz por la parte que vamos en el paper*/

	wi = normalize(light_pos);

	/* Inicio: Generación de muestras */

	for (int i = 0; i < n_samples; i++)
    {
		vec3 sample_e = frag_pos;
        vec4 offset = vec4(sample_e, 1.0);
        // De espacio de vista a espacio de clipping
        offset = projection_matrix * offset;
        offset.xyz /= offset.w;
        // El offset estará en un rango [0:1]
        offset.xyz = offset.xyz * 0.5 + 0.5;
		offset.xy += samples[i].xy;
		xi = texture(g_position, offset.xy).xyz;
		ni = texture(g_normal, offset.xy).xyz;

		float dot_n_w = dot(ni,wi);
		
		if (dot_n_w > 0.0f)
		{
			x = xo - xi;
			r = vec3(length(x));
			w12 = refract(wi, ni, refractive_index);	

			/* Inicio: Parte Difusa */

			ni_ast = calculate_ni_ast(xo, xi, ni);

			xv = xi  + (2 * A * de * ni_ast);
			dv = vec3(length(xo - xv));
			wv = w12 - (2 * (dot(w12, ni_ast)) * ni_ast);
			//dr = sqrt(pow(r, vec3(2.0f)) + pow(zr, vec3(2.0f)));

			cos_beta = -sqrt((pow(r, vec3(2.0f)) - pow(dot(x, w12), 2)) / (pow(r, vec3(2.0f)) + pow(de, vec3(2.0f))));
			miu_0 = dot(-no, w12);
			dr_pow = calculate_dr_pow(r, D, miu_0, de, cos_beta, attenuation_coeff);
			dr = sqrt(dr_pow);

			diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, no);
			diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, no);
			diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;

			Ti = fresnel_t(wi, ni, refractive_index);
			To = fresnel_t(wo, no, refractive_index);

			//diffuse_part = (Ti * diffuse_part_d * dot_n_w * rj) / p;

			diffuse_part = (Ti * diffuse_part_d * dot_n_w);

			Lo += diffuse_part;

			/* Fin: Parte Difusa */
		} 
	}

	Lo *= ((PI * Ll) / n_samples);

	/* Fin: Generación de muestras */	

	color = vec4(Lo * diffuse_reflectance, 1.0f);
}