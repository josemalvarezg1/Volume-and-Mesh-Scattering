#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class material
{
public:
	glm::vec3 scattering_coeff, absorption_coeff, attenuation_coeff, scattering_coeff_prime, attenuation_coeff_prime, diffuse_reflectance;
	glm::vec3 D, effective_transport_coeff, de, zr;
	float refractive_index, c_phi_1, c_phi_2, c_e, A;

	material(glm::vec3 scattering_coeff, glm::vec3 attenuation_coeff, glm::vec3 diffuse_reflectance, float refractive_index);
	~material();

	void precalculate_values(float asymmetry_param_g);
	float calculate_c_phi(float ni);
	float calculate_c_e(float ni);
};

class materials_set
{
public:
	std::vector<material*> materials;
	materials_set();
	~materials_set();
};