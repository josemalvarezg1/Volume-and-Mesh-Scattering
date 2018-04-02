#pragma once
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "GLSLProgram.h"
#include "InterfaceVolume.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class cube
{
public:
	GLuint vboCube, vaoCube, eboCube;
	cube();
	~cube();
	void display();
};

class volume
{
public:
	std::string name;
	GLuint width, height, depth, numOfBits, volumeText;
	glm::quat rotation;
	glm::vec3 translation;
	GLfloat escalation, step;
	volume(std::string path, GLuint width, GLuint height, GLuint depth, GLuint numOfBits);
	~volume();
};

class volumeRender
{
public:
	CGLSLProgram backface, raycasting;
	int screenWidth, screenHeight;
	int indexSelect;
	GLuint frameBuffer, backFaceText, transferFunctionText;
	cube *unitaryCube;
	std::vector<volume*> volumes;
	bool pressVolumeRight, pressVolumeLeft;
	double xReference, yReference;
	interface_volume *volume_interface;
	bool visible_interface;

	volumeRender(int screenWidth, int screenHeight);
	~volumeRender();
	void dropPath(int count, const char** paths);
	bool processPath(std::string path);
	glm::uvec4 getParameters(std::string path);
	void initShaders();
	void scrollVolume(double yoffset);
	bool clickVolume(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, bool type);
	bool poscursorVolume(double x, double y);
	void disableSelect();
	bool intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition);
	void loadTransferFunct(GLfloat data[][4]);
	void UpdateTransferFunction(std::vector<double> points);
	void createBackFaceText();
	bool createFrameBuffer();
	void renderCube(glm::mat4 &MVP);
	void renderCubeRayCast(glm::mat4 &MVP, glm::mat4 &model, glm::vec3 viewPos, glm::vec3 lightPos, bool on, glm::vec3 ambientComp, glm::vec3 diffuseComp, glm::vec3 specularComp);
	void display(glm::mat4 &viewProjection, glm::vec3 viewPos, glm::vec3 lightPos, bool on, glm::vec3 ambientComp, glm::vec3 diffuseComp, glm::vec3 specularComp);
	void changeVolume(int type);
	void resizeScreen(const glm::vec2 screen);

	void update_interface();
};