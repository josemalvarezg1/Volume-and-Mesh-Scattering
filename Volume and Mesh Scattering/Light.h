#pragma once
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

#include "GLSLProgram.h"
#include "InterfaceLight.h"


class light
{
public:
	GLuint vbo, vao, vindex;
	glm::vec3 max_vertex, min_vertex, center, translation, ambient_comp, diffuse_comp, specular_comp;
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> index;
	CGLSLProgram light_program;
	interfaceLight *light_interface;
	bool on, visible_interface;
	GLfloat max_value;

	light();
	~light();
	void init_shaders();
	void set_max_min_value(GLfloat x, GLfloat y, GLfloat z);
	void insert_index(std::string value);
	void calculate_center();
	void load(std::string path);
	void create_vbo();
	void display(glm::mat4 &view_projection);
	void click_light();
	void not_click_light();
	void on_off_light();
	bool update_interface();
};