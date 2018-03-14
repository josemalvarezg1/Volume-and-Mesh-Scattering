#version 330
in vec3 FragPos;
in vec3 Normal;

uniform float assimetry_g;
uniform float scattering_coeff;
uniform float absorption_coeff;
uniform int n_samples;

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
	vec3 inputs = vec3(gl_FragCoord.xyz);
	float rand = random(inputs);
	color = vec4(rand, rand, rand, 1.0);
}