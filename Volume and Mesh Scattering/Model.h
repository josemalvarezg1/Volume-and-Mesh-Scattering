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

class model 
{
public:
	GLuint vbo, vao;
	std::vector<glm::vec3> vertices, normals;
	glm::vec3 max_vertex, min_vertex, center, translation, ambient_comp, diffuse_comp, specular_comp;
	glm::quat rotation;
	GLfloat max_value, shininess;
	GLdouble scale;

	model();
	~model();
	void set_max_min_value(GLfloat x, GLfloat y, GLfloat z);
	std::vector<std::string> process_attribute(std::string value);
	std::vector<std::string> split(std::string s, char delim);
	void calculate_center();
	void load(std::string path);
	void create_vbo();
};