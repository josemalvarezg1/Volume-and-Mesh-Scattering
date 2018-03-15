#version 330
in vec3 FragPos;
in vec3 Normal;

uniform float assimetry_g;
uniform int n_samples;
uniform vec3 samples[64];
uniform mat4 projection_matrix;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

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
		float r = length(x0 - xi);
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
	}
	color = texture(gPosition, vec2(0.1, 0.1));
}