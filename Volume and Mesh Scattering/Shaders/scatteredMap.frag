#version 330
in vec3 FragPos;
in vec3 Normal;

uniform float assimetry_g;
uniform int n_samples;
uniform vec3 samples[64];
uniform mat4 projection_matrix;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform float refractive_index;

const float PI = 3.1415926535897932384626433832795;

out vec4 color;

uint hash(uint x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

uint hash(uvec3 v) {
	return hash(v.x ^ hash(v.y) ^ hash(v.z));
}

// Retorna en un rango [0:1]
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

float calculate_c_phi(float refractive_index) {
	float C1 = 0.0;
	if (refractive_index < 1)
		C1 = 0.919317 - 3.4793 * refractive_index + 6.75335 * pow(refractive_index, 2) - 7.80989 * pow(refractive_index, 3) + 4.98554 * pow(refractive_index, 4) - 1.36881 * pow(refractive_index, 5);
	else
		C1 = -9.23372 + 22.2272 * refractive_index - 20.9292 * pow(refractive_index, 2) + 10.2291 * pow(refractive_index, 3) - 2.54396 * pow(refractive_index, 4) + 0.254913 * pow(refractive_index, 5);
	return 1 / 4 * (1 - C1);
}

float calculate_c_e(float refractive_index) {
	float C2 = 0.0;
	if (refractive_index < 1)
		C2 = 0.828421 - 2.62051 * refractive_index + 3.36231 * pow(refractive_index, 2) - 1.95284 * pow(refractive_index, 3) + 0.236494 * pow(refractive_index, 4) + 0.145787 * pow(refractive_index, 5);
	else
		C2 = -1641.1 + 135.926 / pow(refractive_index, 3) - 656.175 / pow(refractive_index, 2) + 1376.53 / refractive_index + 1213.67 * refractive_index - 568.556 * pow(refractive_index, 2) + 164.798 * pow(refractive_index, 3) - 27.0181 * pow(refractive_index, 4) + 1.91826 * pow(refractive_index, 5);
	return 1 / 2 * (1 - C2);
}

vec3 diffuse_part_prime(vec3 x, vec3 w12, float r, vec3 transport_coeff, float c_phi1, float c_phi2, float c_e, vec3 D, vec3 Normal) {
	return (1 / (4 * c_phi2)) * 2.4674011 * (exp(-transport_coeff) / pow(r, 3))
		    * (c_phi1 * (pow(r, 2) / D + 3 * (1 + transport_coeff) * dot(x, w12))
			- c_e * (3 * D * (1 + transport_coeff) * dot(w12, Normal)
			- ((1 + transport_coeff) + 3 * D * (3 * (1 + transport_coeff) + pow(transport_coeff, 2)) / pow(r, 2) * dot(x, w12)) * dot(x, Normal)));
}

void main() {
	// Skin 1 BRDF
	vec3 scattering_coeff = vec3(0.74, 0.88, 1.01);
	vec3 absorption_coeff = vec3(0.032, 0.17, 0.48);
	float rand1 = random(vec3(gl_FragCoord.xyz));
	float rand2 = random(vec3(gl_FragCoord.zxy));
	vec3 transport_coeff = vec3(0.0, 0.0, 0.0);
	transport_coeff.x = sqrt(3 * absorption_coeff.x * (absorption_coeff.x + (1 - assimetry_g) * scattering_coeff.x));
	transport_coeff.y = sqrt(3 * absorption_coeff.y * (absorption_coeff.y + (1 - assimetry_g) * scattering_coeff.y));
	transport_coeff.z = sqrt(3 * absorption_coeff.z * (absorption_coeff.z + (1 - assimetry_g) * scattering_coeff.z));

	vec3 x0 = FragPos;
	vec3 randomVec = vec3(random(vec3(gl_FragCoord.xyz)), random(vec3(gl_FragCoord.yxz)), random(vec3(gl_FragCoord.zyx)));
	vec3 tangent = normalize(randomVec - Normal * dot(randomVec, Normal));
	vec3 bitangent = cross(Normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, Normal);
	int size = 64;
	float radius = 0.5;


	// S'd
	vec3 D = 1 / 3 * (scattering_coeff * (1 - assimetry_g) + absorption_coeff);
	float c_phi1 = calculate_c_phi(refractive_index);
	float c_phi2 = calculate_c_phi(1 / refractive_index);
	float c_e = calculate_c_e(refractive_index);

	for (int i = 0; i < size; i++)
	{
		vec3 sample = TBN * samples[i];
		sample = FragPos + sample * radius;
		vec4 offset = vec4(sample, 1.0);
		offset = projection_matrix * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5; // Rango 0.0 - 1.0
		vec3 pos_light = -texture(gPosition, offset.xy).xyz;
		vec3 normal_light = -texture(gNormal, offset.xy).xyz;
		vec3 xi = pos_light;
		vec3 x = x0 - xi;
		float r = length(x);
		//vec3 w12 = refract(normalize(xi - cameraPos), normalize(ni), refractive_index);
		vec3 rj = vec3(0.0, 0.0, 0.0);
		rj.x = -log(rand1) / transport_coeff.x;
		rj.y = -log(rand1) / transport_coeff.y;
		rj.z = -log(rand1) / transport_coeff.z;
		float aj = 2 * PI * rand2;
		vec3 p = vec3(0.0, 0.0, 0.0);
		p.x = transport_coeff.x * exp(-transport_coeff.x * r) * (1 / (2 * PI));
		p.y = transport_coeff.y * exp(-transport_coeff.y * r) * (1 / (2 * PI));
		p.z = transport_coeff.z * exp(-transport_coeff.z * r) * (1 / (2 * PI));

		// A partir de aquí se realiza la sumatoria

		vec3 diffuse_part_prime1 = diffuse_part_prime(x, w12, r, transport_coeff * r,
									   c_phi1, c_phi2, c_e, D, Normal);

	}
	color = texture(gPosition, vec2(0.1, 0.1));
}