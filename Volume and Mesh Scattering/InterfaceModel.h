#pragma once
#include <GL/glew.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class interfaceModel
{
private:
	static interfaceModel *userInterface;
	TwBar *modelInterface;
	interfaceModel();

public:
	glm::vec3 translation;
	glm::quat rotation;
	GLfloat scale, shininess;

	static interfaceModel * instance();
	~interfaceModel();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
