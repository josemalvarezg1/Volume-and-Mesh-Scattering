#pragma once
#include <AntTweakBar.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>

class interface_light
{
private:
	static interface_light *user_interface;
	TwBar *light_interface;
	interface_light();

public:
	glm::vec3 translation, ambient_comp, diffuse_comp, specular_comp;
	bool on;

	static interface_light * instance();
	~interface_light();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
