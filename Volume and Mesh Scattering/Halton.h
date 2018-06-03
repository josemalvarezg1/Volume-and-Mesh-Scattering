#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

const float PI = 3.1415926535897932384626433832795;

class halton
{
public:
	std::vector<glm::vec3> camera_positions;
	std::vector<glm::vec2> samples;

	halton();
	~halton();
	double halton_sequence(int index, int base);
	glm::vec2 halton_point_circle(glm::vec2 halton_point);
	int negative_positive();
	void generate_orthographic_cameras(int num_of_orto_cameras);
	void generate_samples(float sigma_tr, float radius, int num_of_samples_per_frag);
	void add_new_camera(int index);
};