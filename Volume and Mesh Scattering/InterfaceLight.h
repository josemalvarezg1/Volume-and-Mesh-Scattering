#pragma once
#include <AntTweakBar.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>

class interfaceLight
{
private:
	static interfaceLight *userInterface;
	TwBar *lightInterface;
	interfaceLight();

public:
	glm::vec3 translation, ambient_comp, diffuse_comp, specular_comp;
	bool on;

	static interfaceLight * instance();
	~interfaceLight();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
