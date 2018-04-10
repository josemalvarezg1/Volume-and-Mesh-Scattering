#pragma once
#include <GL/glew.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

typedef enum { Patata, Mármol, Piel, Leche, Crema, Ninguno } material_m;

class interface_model
{
private:
	static interface_model *user_interface;
	TwBar *model_interface;
	interface_model();

public:
	glm::vec3 translation;
	glm::quat rotation;
	GLfloat scale, asymmetry_param_g, q, radius, epsilon;
	material_m current_material;

	static interface_model * instance();
	~interface_model();

	void reshape(int g_width, int g_height);
	void show();
	void hide();
};
