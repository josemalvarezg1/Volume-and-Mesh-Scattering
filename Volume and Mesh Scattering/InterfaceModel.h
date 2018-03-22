#pragma once
#include <GL/glew.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

typedef enum { Patata, Mármol, Piel, Leche, Ninguno } material;

class interfaceModel
{
private:
	static interfaceModel *userInterface;
	TwBar *modelInterface;
	interfaceModel();

public:
	glm::vec3 translation;
	glm::quat rotation;
	GLfloat scale, shininess, asymmetry_param_g, refractive_index, radius;
	material current_material;

	static interfaceModel * instance();
	~interfaceModel();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
