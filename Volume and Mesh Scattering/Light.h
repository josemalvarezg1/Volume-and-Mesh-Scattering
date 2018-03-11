#pragma once
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLSLProgram.h"
#include <fstream>

class light
{
public:
	glm::vec3 maximus, minimous, translation, ambientComp, diffuseComp, specularComp;
	GLuint vbo, vao, vindex;
	std::vector<glm::vec3> vertex;
	std::vector<GLuint> index;
	CGLSLProgram lightP;
	bool on, viewInterface;

	light();
	~light();
	void createVBO();
	void display(glm::mat4 &viewProjection);
	void load(std::string path);
	void insertIndex(std::string value);
	void initShaders();
	//bool clickLight(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, int screenWidth, int screenHeight);
	bool intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, int screenWidth, int screenHeight);
	void onOffLight();
	void updateInterface();
};