#version 330
in vec3 FragPos;
in vec3 Normal;

uniform float assimetry_g;
uniform float scattering_coeff;
uniform float absorption_coeff;
uniform int n_samples;
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

void main() {
	float rand1 = random(vec3(gl_FragCoord.xyz));
	float rand2 = random(vec3(gl_FragCoord.zxy));
	float transport_coeff = sqrt(3 * absorption_coeff * (absorption_coeff + (1 - assimetry_g) * scattering_coeff));
	float rj = -log(rand1) / transport_coeff;
	float aj = 2 * PI * rand2;
	float r = 1.0; // Revisar este valor
	float p = transport_coeff * exp(-transport_coeff * r) * (1 / (2 * PI));
	color = vec4(rand2, rand2, rand2, 1.0);
}