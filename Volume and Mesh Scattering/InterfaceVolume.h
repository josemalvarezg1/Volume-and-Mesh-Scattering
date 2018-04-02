#pragma once
#include <GL/glew.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class interface_volume
{
private:
	static interface_volume *user_interface;
	TwBar *volume_interface;
	interface_volume();

public:
	glm::vec3 translation;
	glm::quat rotation;
	GLfloat scale;

	static interface_volume * instance();
	~interface_volume();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
