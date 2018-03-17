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

uniform int n_samples;
uniform vec3 samples[64];
uniform mat4 projection_matrix;
uniform sampler2D g_position;
uniform sampler2D g_normal;


const float PI = 3.1415926535897932384626433832795;

out vec4 color;

//uint hash(uint x) 
//{
//	x += (x << 10u);
//	x ^= (x >> 6u);
//	x += (x << 3u);
//	x ^= (x >> 11u);
//	x += (x << 15u);
//	return x;
//}

//uint hash(uvec3 v) 
//{
//	return hash(v.x ^ hash(v.y) ^ hash(v.z));
//}

// Retorna en un rango [0:1]
//float floatConstruct(uint m) {
//	const uint ieeeMantissa = 0x007FFFFFu;
//	const uint ieeeOne = 0x3F800000u;

//	m &= ieeeMantissa;
//	m |= ieeeOne;

//	float  f = uintBitsToFloat(m);
//	return f - 1.0;
//}

//float random(vec3 v) {
//	return floatConstruct(hash(floatBitsToUint(v)));
//}

vec3 calculate_ni_ast(vec3 xo, vec3 xi, vec3 ni)
{
	if(xo = xi)
		return ni;
	else
	{
		vec3 factor_1, factor_2;
		factor_1 = normalize(xo - xi);
		factor_2 = normalize(cross(ni, xo - xi));
		return cross(factor_1, factor_2);
	}
}

float calculate_c_phi(float ni) 
{
	float C1 = 0.0;
	if (ni < 1)
		C1 = 0.919317 - 3.4793 * ni + 6.75335 * pow(ni, 2) - 7.80989 * pow(ni, 3) + 4.98554 * pow(ni, 4) - 1.36881 * pow(ni, 5);
	else
		C1 = -9.23372 + 22.2272 * ni - 20.9292 * pow(ni, 2) + 10.2291 * pow(ni, 3) - 2.54396 * pow(ni, 4) + 0.254913 * pow(ni, 5);
	return 1 / 4 * (1 - C1);
}

float calculate_c_e(float ni) 
{
	float C2 = 0.0;
	if (ni < 1)
		C2 = 0.828421 - 2.62051 * ni + 3.36231 * pow(ni, 2) - 1.95284 * pow(ni, 3) + 0.236494 * pow(ni, 4) + 0.145787 * pow(ni, 5);
	else
		C2 = -1641.1 + 135.926 / pow(ni, 3) - 656.175 / pow(ni, 2) + 1376.53 / ni + 1213.67 * ni - 568.556 * pow(ni, 2) + 164.798 * pow(ni, 3) - 27.0181 * pow(ni, 4) + 1.91826 * pow(ni, 5);
	return 1 / 2 * (1 - C2);
}

vec3 diffuse_part_prime(vec3 x, vec3 w12, float dr, float c_phi2, vec3 effective_transport_coeff, vec3 r, float c_phi1, vec3 D, float c_e, vec3 no) 
{
	vec3 factor_1, factor_2, factor_3, factor_4;
	factor_1 = (1 / (4 * c_phi2)) * 2.4674011 * (exp(-effective_transport_coeff) / pow(r, 3));
	factor_2 = c_phi1 * (pow(r, 2) / D + 3 * (1 + effective_transport_coeff) * dot(x, w12));
	factor_3 = 3 * D * (1 + effective_transport_coeff) * dot(w12, no);
	factor_4 = ((1 + effective_transport_coeff) + 3 * D * (3 * (1 + effective_transport_coeff) + pow(effective_transport_coeff, 2)) / pow(r, 2) * dot(x, w12)) * dot(x, no);
	return factor_1 * (factor_2 - c_e * (factor_3 - factor_4));
}

void main() 
{
	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, w12, rj, alphaj, p, A, de, ni_ast, zr, xv, dv, wv;
	vec3 scattering_coeff, absorption_coeff, attenuation_coeff, albedo, scattering_coeff_prime, attenuation_coeff_prime, albedo_prime;
	vec3 D, effective_transport_coeff, diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
	vec3 cos_beta, z_prime, R, T;
	float xi_1, xi_2, c_phi_1, c_phi_2, c_e;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - frag_pos);

	Lo = vec3(0.0);
	Ll = light_diff; 		/*Hasta ahora una sola luz por la parte que vamos en el paper*/
	
	wi = normalize(light_pos);


	/*Estas variables las debemos traer precalculadas para no hacer esto en el shader*/

	scattering_coeff = vec3(0.74, 0.88, 1.01);
	absorption_coeff = vec3(0.032, 0.17, 0.48);
	attenuation_coeff = scattering_coeff + absorption_coeff;
	albedo = scattering_coeff / attenuation_coeff;

	scattering_coeff_prime = scattering_coeff * (1 - asymmetry_param_g);
	attenuation_coeff_prime = scattering_coeff_prime + absorption_coeff;
	albedo_prime = scattering_coeff_prime / attenuation_coeff_prime;

	D = 1 / (3 * attenuation_coeff_prime);
	effective_transport_coeff = sqrt(absorption_coeff / D);

	/*Estas variables las debemos traer precalculadas para no hacer esto en el shader*/
	

	xi_1 = random(vec3(gl_FragCoord.xyz));
	xi_2 = random(vec3(gl_FragCoord.zxy));
	

	// vec3 randomVec = vec3(random(vec3(gl_FragCoord.xyz)), random(vec3(gl_FragCoord.yxz)), random(vec3(gl_FragCoord.zyx)));
	// vec3 tangent = normalize(randomVec - Normal * dot(randomVec, Normal));
	// vec3 bitangent = cross(Normal, tangent);
	// mat3 TBN = mat3(tangent, bitangent, Normal);
	// int size = 64;
	// float radius = 0.5;


	/* S'd */

	c_phi_1 = calculate_c_phi(refractive_index);
	c_phi_2 = calculate_c_phi(1 / refractive_index);
	c_e = calculate_c_e(refractive_index);

	A = (1 - c_e) / (2 * c_phi_1);
	de = 2.131 * D * sqrt(albedo_prime);
	zr = 3 * D;

	/* S'd */

	for (int i = 0; i < size; i++)
	{
		// vec3 sample = TBN * samples[i];
		// sample = FragPos + sample * radius;
		// vec4 offset = vec4(sample, 1.0);
		// offset = projection_matrix * offset;
		// offset.xyz /= offset.w;
		// offset.xyz = offset.xyz * 0.5 + 0.5;
		xi = texture(g_position, offset.xy).xyz;
		ni = texture(g_normal, offset.xy).xyz;
		
		x = xo - xi;
		r = length(x);
		w12 = refract(normalize(xi - camera_pos), normalize(ni), refractive_index);
		
		rj = = -log(xi_1) / effective_transport_coeff;
		alphaj = 2.0 * PI * xi_2;

		p = effective_transport_coeff * exp(-effective_transport_coeff * r) * (1 / (2 * PI));

		/* Parte Difusa */

		ni_ast = calculate_ni_ast(xo, xi, ni);

		xv = xi  + (2 * A * de * ni_ast);
		dv = length(xo - xv);
		wv = w12 - (2 * (dot(w12, ni_ast)) * ni_ast);
		dr = sqrt(pow(r, 2) + pow(zr, 2));

		diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, c_phi2, effective_transport_coeff, r, c_phi1, D, c_e, no);
		diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, c_phi2, effective_transport_coeff, r, c_phi1, D, c_e, no);
		diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;

		// Revisar Fresnel
		cos_beta = -sqrt((pow(r, 2) - pow(dot(x, w12), 2)) / (pow(r, 2) + pow(de, 2)));
		z_prime = D * length(dot(-w12, no));
		R = sqrt(pow(r, 2) + pow(z_prime, 2) + pow(de, 2) - 2 * z_prime * de * cos_beta);
		T = 1 - R;

		/* Fin Parte Difusa */

		

	}
	color = texture(gPosition, vec2(0.1, 0.1));
}