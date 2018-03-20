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
uniform vec3 scattering_coeff;
uniform vec3 absorption_coeff;
uniform vec3 diffuse_reflectance;
uniform mat4 projection_matrix;

uniform int n_samples;
uniform vec3 samples[64];
uniform mat4 model_matrix;
uniform sampler2D g_position;
uniform sampler2D g_normal;

// Valores pre-calculados
uniform vec3 scattering_coeff_prime;
uniform vec3 attenuation_coeff;
uniform vec3 albedo;
uniform vec3 attenuation_coeff_prime;
uniform vec3 albedo_prime;
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

uint hash(uint x) 
{
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

uint hash(uvec3 v) 
{
	return hash(v.x ^ hash(v.y) ^ hash(v.z));
}

 //Retorna en un rango [0:1]
float floatConstruct(uint m) {
	const uint ieeeMantissa = 0x007FFFFFu;
	const uint ieeeOne = 0x3F800000u;

	m &= ieeeMantissa;
	m |= ieeeOne;

	float  f = uintBitsToFloat(m);
	return f - 1.0;
}

float random(vec3 v) {
	return floatConstruct(hash(floatBitsToUint(v)));
}

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

vec3 diffuse_part_prime(vec3 x, vec3 w12, vec3 r, float c_phi2, vec3 effective_transport_coeff, float c_phi1, vec3 D, float c_e, vec3 no) 
{
	vec3 effec_r, one_effec_r, factor_1, factor_2, factor_3, factor_4;
	float x_dot_w12, w12_dot_no, x_dot_no;
	effec_r = effective_transport_coeff * r;
	one_effec_r = vec3(1.0f) + effec_r;
	x_dot_w12 = dot(x, w12);
    w12_dot_no = dot(w12, no);
    x_dot_no = dot(x, no);
	factor_1 = (1 / (4 * c_phi2)) * 2.4674011 * (exp(-effec_r) / pow(r, vec3(3.0f)));
	factor_2 = c_phi1 * ((r * r) / D + 3 * one_effec_r * x_dot_w12);
	factor_3 = 3 * D * one_effec_r * w12_dot_no;
	factor_4 = (one_effec_r + 3 * D * (3 * one_effec_r + effec_r * effec_r) / (r * r) * x_dot_w12) * x_dot_no;
	return factor_1 * (factor_2 - c_e * (factor_3 - factor_4));
}

float fresnel_t(vec3 I, vec3 N, float ior) 
{ 
	float cosi, etai, etat, aux, sint;
	cosi = clamp(-1.0f, 1.0f, dot(I, N));
    etai = 1.0f;
    etat = ior;
    if (cosi > 0.0f)
	{
		aux = etai;
		etai = etat;
		etat = aux;
	}
    sint = etai / etat * sqrt(max(0.0f, 1.0f - cosi * cosi)); 
    if (sint >= 1.0f)
        return 1.0f;
	float cost, Rs, Rp;
    cost = sqrt(max(0.0f, 1.0f - sint * sint)); 
    cosi = abs(cosi); 
    Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
    Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
    return (Rs * Rs + Rp * Rp) / 2.0f;
} 

void main() 
{
	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, dr, dr_pow, w12, rj, p, ni_ast, xv, dv, wv;
	vec3 diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
	vec3 cos_beta, z_prime, R, T, diffuse_part;
	float xi_1, xi_2, miu_0, Ti, To, alphaj;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - frag_pos);

	Lo = vec3(0.0f);
	Ll = light_diff.xyz; 		/*Hasta ahora una sola luz por la parte que vamos en el paper*/

	wi = normalize(light_pos);
	
	xi_1 = random(vec3(gl_FragCoord.xyz));
	xi_2 = random(vec3(gl_FragCoord.zxy));	

	/* Inicio: Generación de muestras */
	float radius = 1.0f / 32.0f;

	for (int i = 0; i < n_samples; i++)
    {
        vec3 sample_e = frag_pos + samples[i] * radius; 
        vec4 offset = vec4(sample_e, 1.0);
        // De espacio de vista a espacio de clipping
        offset = projection_matrix * offset;
        offset.xyz /= offset.w;
        // El offset estará en un rango [0:1]
        offset.xyz = offset.xyz * 0.5 + 0.5;

		xi = texture(g_position, offset.xy).xyz;
		ni = texture(g_normal, offset.xy).xyz;

		float dot_n_w = dot(ni,wi);
		
		if (dot_n_w > 0.0f)
		{
			x = xo - xi;
			r = vec3(length(x));
			w12 = refract(wi, ni, 1.0f / refractive_index);
			
			rj = -log(xi_1) / effective_transport_coeff;
			alphaj = 2.0 * PI * xi_2;

			p = effective_transport_coeff * exp(-effective_transport_coeff * r) * (1 / (2 * PI));

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

			diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, c_phi_2, effective_transport_coeff, c_phi_1, D, c_e, no);
			diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, c_phi_2, effective_transport_coeff, c_phi_1, D, c_e, no);
			diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;

			Ti = fresnel_t(wi, ni, refractive_index);
			To = fresnel_t(wo, no, refractive_index);

			diffuse_part = Ti * diffuse_part_d;

			//diffuse_part = vec3(Ti);

			Lo += diffuse_part;

			/* Fin: Parte Difusa */
		} 
	}

	/* Fin: Generación de muestras */	

	color = vec4((Lo / n_samples) * diffuse_reflectance, 1.0f);
}