#pragma once
#define BUFFER_OFFSET(offset) ((char*)NULL + (offset))
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include "InterfaceModel.h"

class mesh
{
public:
	GLuint vbo, vao;
	glm::quat rotation;
	GLfloat scale, max_value, shininess;
	std::vector<glm::vec3> vertices, normals;
	glm::vec3 max_vertex, min_vertex, center, translation, ambient_comp, diffuse_comp, specular_comp;
	
	mesh();
	~mesh();
	void set_max_min_value(GLfloat x, GLfloat y, GLfloat z);
	std::vector<std::string> process_attribute(std::string value);
	std::vector<std::string> split(std::string s, char delim);
	void calculate_center();
	void load(std::string path);
	void create_vbo();
};

class meshSet {
public:
	bool visible_interface;

	meshSet();
	~meshSet();
	std::vector<mesh*> mesh_models;
	interfaceModel *model_interface;
	void click_model(int selectedModel);
	void not_click_model();
	void update_interface(int selectedModel);
};

	