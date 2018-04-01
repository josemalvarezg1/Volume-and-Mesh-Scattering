#include "Material.h"

material::material(glm::vec3 scattering_coeff, glm::vec3 absorption_coeff, glm::vec3 diffuse_reflectance, float refractive_index)
{
	this->scattering_coeff = scattering_coeff;
	this->absorption_coeff = absorption_coeff;
	this->diffuse_reflectance = diffuse_reflectance;
	this->refractive_index = refractive_index;
}

material::~material()
{
}

void material::precalculate_values(float asymmetry_param_g)
{
	this->scattering_coeff_prime = (1.0f - asymmetry_param_g) * this->scattering_coeff;
	this->attenuation_coeff = this->scattering_coeff + this->absorption_coeff;
	this->attenuation_coeff_prime = this->scattering_coeff_prime + this->absorption_coeff;

	this->D = glm::vec3(1.0f) / (glm::vec3(3.0f) * this->attenuation_coeff_prime);
	this->effective_transport_coeff = sqrt(this->absorption_coeff / this->D);

	this->c_phi_1 = this->calculate_c_phi(this->refractive_index);
	this->c_phi_2 = calculate_c_phi(1.0f / this->refractive_index);
	this->c_e = calculate_c_e(this->refractive_index);

	this->A = (1.0f - this->c_e) / (2.0f * this->c_phi_1);
	this->de = 2.131f * this->D * sqrt(this->attenuation_coeff_prime / this->scattering_coeff_prime);
	this->zr = 3.0f * this->D;
}

float material::calculate_c_phi(float ni)
{
	float C1 = 0.0f;
	if (ni < 1.0f)
		C1 = 0.919317f - 3.4793f * ni + 6.75335f * pow(ni, 2) - 7.80989f * pow(ni, 3) + 4.98554f * pow(ni, 4) - 1.36881f * pow(ni, 5);
	else
		C1 = -9.23372f + 22.2272f * ni - 20.9292f * pow(ni, 2) + 10.2291f * pow(ni, 3) - 2.54396f * pow(ni, 4) + 0.254913f * pow(ni, 5);
	return 1.0f / 4.0f * (1.0f - C1);
}

float material::calculate_c_e(float ni)
{
	float C2 = 0.0f;
	if (ni < 1.0f)
		C2 = 0.828421f - 2.62051f * ni + 3.36231f * pow(ni, 2) - 1.95284f * pow(ni, 3) + 0.236494f * pow(ni, 4) + 0.145787f * pow(ni, 5);
	else
		C2 = -1641.1f + 135.926f / pow(ni, 3) - 656.175f / pow(ni, 2) + 1376.53f / ni + 1213.67f * ni - 568.556f * pow(ni, 2) + 164.798f * pow(ni, 3) - 27.0181f * pow(ni, 4) + 1.91826f * pow(ni, 5);
	return 1.0f / 2.0f * (1.0f - C2);
}

materials_set::materials_set()
{
}

materials_set::~materials_set()
{
	this->materials.clear();
}