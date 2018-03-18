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
uniform mat4 projection_matrix;

uniform int n_samples;
uniform vec3 samples[64];
uniform mat4 model_matrix;
uniform sampler2D g_position;
uniform sampler2D g_normal;


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
	if(xo == xi)
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

float calculate_c_phi(float ni) 
{
	float C1 = 0.0f;
	if (ni < 1.0f)
		C1 = 0.919317f - 3.4793f * ni + 6.75335f * pow(ni, 2) - 7.80989f * pow(ni, 3) + 4.98554f * pow(ni, 4) - 1.36881f * pow(ni, 5);
	else
		C1 = -9.23372f + 22.2272f * ni - 20.9292f * pow(ni, 2) + 10.2291f * pow(ni, 3) - 2.54396f * pow(ni, 4) + 0.254913f * pow(ni, 5);
	return 1.0f / 4.0f * (1.0f - C1);
}

float calculate_c_e(float ni) 
{
	float C2 = 0.0f;
	if (ni < 1.0f)
		C2 = 0.828421f - 2.62051f * ni + 3.36231f * pow(ni, 2) - 1.95284f * pow(ni, 3) + 0.236494f * pow(ni, 4) + 0.145787f * pow(ni, 5);
	else
		C2 = -1641.1f + 135.926f / pow(ni, 3) - 656.175f / pow(ni, 2) + 1376.53f / ni + 1213.67f * ni - 568.556f * pow(ni, 2) + 164.798f * pow(ni, 3) - 27.0181f * pow(ni, 4) + 1.91826f * pow(ni, 5);
	return 1.0f / 2.0f * (1.0f - C2);
}

vec3 diffuse_part_prime(vec3 x, vec3 w12, vec3 dr, float c_phi2, vec3 effective_transport_coeff, float c_phi1, vec3 D, float c_e, vec3 no, vec3 dr_pow) 
{
	vec3 one_effec_dr, factor_1, factor_2, factor_3, factor_4;
	one_effec_dr = (vec3(1.0f) + effective_transport_coeff * dr);
	factor_1 = (1 / (4 * c_phi2)) * 2.4674011 * (exp(-effective_transport_coeff) / pow(dr, vec3(3.0f)));
	factor_2 = c_phi1 * (dr_pow / D + 3 * (one_effec_dr) * dot(x, w12));
	factor_3 = 3 * D * (one_effec_dr) * dot(w12, no);
	factor_4 = ((one_effec_dr) + 3 * D * (3 * (one_effec_dr) + (pow(effective_transport_coeff, vec3(2.0f)) * dr_pow)) / dr_pow * dot(x, w12)) * dot(x, no);
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
	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, dr, dr_pow, w12, rj, p, de, ni_ast, zr, xv, dv, wv;
	vec3 scattering_coeff, absorption_coeff, attenuation_coeff, albedo, scattering_coeff_prime, attenuation_coeff_prime, albedo_prime;
	vec3 D, effective_transport_coeff, diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
	vec3 cos_beta, z_prime, R, T, diffuse_part;
	vec2 offset;
	float xi_1, xi_2, c_phi_1, c_phi_2, c_e, miu_0, Ti, To, A, alphaj;

	xo = frag_pos;
	no = normalize(frag_normal);
	wo = normalize(camera_pos - frag_pos);

	Lo = vec3(0.0f);
	Ll = light_diff.xyz; 		/*Hasta ahora una sola luz por la parte que vamos en el paper*/

	wi = normalize(light_pos);

	/*Estas variables las debemos traer precalculadas para no hacer esto en el shader*/

	scattering_coeff_prime = vec3(0.68f, 0.70f, 0.55f);
	scattering_coeff = scattering_coeff_prime / (1.0f - asymmetry_param_g);
	absorption_coeff = vec3(0.0024f, 0.0090f, 0.12f);
	attenuation_coeff = scattering_coeff + absorption_coeff;
	albedo = scattering_coeff / attenuation_coeff;

	
	attenuation_coeff_prime = scattering_coeff_prime + absorption_coeff;
	albedo_prime = scattering_coeff_prime / attenuation_coeff_prime;

	D = 1 / (3 * attenuation_coeff_prime);
	effective_transport_coeff = sqrt(absorption_coeff / D);

	/*Estas variables las debemos traer precalculadas para no hacer esto en el shader*/
	
	xi_1 = random(vec3(gl_FragCoord.xyz));
	xi_2 = random(vec3(gl_FragCoord.zxy));

	c_phi_1 = calculate_c_phi(refractive_index);
	c_phi_2 = calculate_c_phi(1 / refractive_index);
	c_e = calculate_c_e(refractive_index);

	A = (1.0f - c_e) / (2.0f * c_phi_1);
	de = 2.131f * D * sqrt(albedo_prime);
	zr = 3.0f * D;

	/* Generacion de muestras */

    //vec3 randomVec = vec3(xi_1, xi_2, random(vec3(gl_FragCoord.yxz)));
    //vec3 tangent = normalize(randomVec - frag_normal * dot(randomVec, frag_normal));
	float radius = 0.5;

	for(int i = 0; i < n_samples; i++)
    {
        vec3 sample_e = frag_pos + samples[i] * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample_e, 1.0);
        offset = projection_matrix * offset; // from view to clip-space
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		xi = texture(g_position, offset.xy).xyz;
		ni = texture(g_normal, offset.xy).xyz;
		
		x = xo - xi;
		r = vec3(length(x));
		w12 = refract(wi, ni, refractive_index);
		
		rj = -log(xi_1) / effective_transport_coeff;
		alphaj = 2.0 * PI * xi_2;

		p = effective_transport_coeff * exp(-effective_transport_coeff * r) * (1 / (2 * PI));

		/* Parte Difusa */

		ni_ast = calculate_ni_ast(xo, xi, ni);

		xv = xi  + (2 * A * de * ni_ast);
		dv = vec3(length(xo - xv));
		wv = w12 - (2 * (dot(w12, ni_ast)) * ni_ast);
		//dr = sqrt(pow(r, vec3(2.0f)) + pow(zr, vec3(2.0f)));

		cos_beta = -sqrt((pow(r, vec3(2.0f)) - pow(dot(x, w12), 2)) / (pow(r, vec3(2.0f)) + pow(de, vec3(2.0f))));
		miu_0 = dot(-no, w12);
		dr_pow = calculate_dr_pow(r, D, miu_0, de, cos_beta, attenuation_coeff);
		dr = sqrt(dr_pow);

		diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, c_phi_2, effective_transport_coeff, c_phi_1, D, c_e, no, dr_pow);
		diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, c_phi_2, effective_transport_coeff, c_phi_1, D, c_e, no, pow(dv, vec3(2.0f)));
		diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;

		Ti = fresnel_t(wi, ni, refractive_index);
		To = fresnel_t(wo, no, refractive_index);

		//diffuse_part = Ti * diffuse_part_d * To;

		diffuse_part = Ti * diffuse_part_d;

		Lo += diffuse_part * vec3(0.77f, 0.62f, 0.21f);

		//Lo += ni;
	
		/* Fin Parte Difusa */
	}

	/* Generacion de muestras */
	

	color = vec4(Lo / n_samples, 1.0f);
}