#include "Halton.h"

halton::halton() 
{
}

halton::~halton()
{
}

double halton::halton_sequence(int index, int base)
{
	double f, r;
	f = 1.0;
	r = 0.0;
	while (index > 0)
	{
		f = f / base;
		r = r + f * (index % base);
		index = index / base;
	}
	return r;
}

glm::vec2 halton::halton_point_circle(glm::vec2 halton_point)
{
	double x, y;
	x = sqrt(1 - pow(2 * halton_point.x - 1, 2)) * cos(2 * PI * halton_point.y);
	y = sqrt(1 - pow(2 * halton_point.x - 1, 2)) * sin(2 * PI * halton_point.y);
	return glm::vec2(x, y);
}

int halton::negative_positive()
{
	if (rand() % 2)
		return 1;
	else
		return -1;
}

void halton::generate_orthographic_cameras(int num_of_orto_cameras)
{
	double x_pos, y_pos, z_pos;
	for (int i = 0; i < num_of_orto_cameras; i++)
	{
		x_pos = this->halton_sequence(i, 2) + (15.0f * this->negative_positive());
		y_pos = this->halton_sequence(i, 3) + (15.0f * this->negative_positive());
		z_pos = this->halton_sequence(i, 7) + (15.0f * this->negative_positive());
		this->camera_positions.push_back(glm::vec3(x_pos, y_pos, z_pos));
	}
}

void halton::generate_samples(float sigma_tr, float radius, int num_of_samples_per_frag)
{
	int i = 1, accepted = 0;
	float rad, expon, zeta;
	glm::vec2 halton_point, point;
	srand(0);
	samples.clear();
	while (accepted < num_of_samples_per_frag)
	{
		halton_point = glm::vec2(this->halton_sequence(i, 2), this->halton_sequence(i, 3));
		point = this->halton_point_circle(halton_point);
		rad = glm::length(point) * radius;
		expon = exp(-sigma_tr * rad);
		zeta = rand() / ((float)(RAND_MAX));
		if (zeta < expon)
		{
			this->samples.push_back(radius * point);
			accepted++;
		}
		i++;
	}
}

void halton::add_new_camera(int index) {
	double x_pos, y_pos, z_pos;
	x_pos = this->halton_sequence(index, 2) + (15.0f * this->negative_positive());
	y_pos = this->halton_sequence(index, 3) + (15.0f * this->negative_positive());
	z_pos = this->halton_sequence(index, 7) + (15.0f * this->negative_positive());
	this->camera_positions.push_back(glm::vec3(x_pos, y_pos, z_pos));
}