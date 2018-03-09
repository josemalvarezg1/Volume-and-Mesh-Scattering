#pragma once
#define BUFFER_OFFSET(offset) ((char*)NULL + (offset))
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	GLuint vbo, vao, vindex;
	std::vector<glm::vec3> vertices, normals;
	glm::vec3 max_vertex, min_vertex, center, translation, ambient_comp, diffuse_comp, specular_comp;
	glm::vec4 rotation;
	GLfloat max_value, shininess;
	GLdouble scale;

	model();
	//~model();
	void load(std::string path);
	void create_vbo();
	void set_max_min_value(GLfloat x, GLfloat y, GLfloat z);
	void calculate_center();
	void triangulate(std::vector<unsigned int> aux_index_vertices, std::vector<unsigned int> aux_index_normals, std::vector<glm::uvec3> &index_vertices, std::vector<glm::uvec3> &index_normals);

	void setQuat(const float *eje, float angulo, float *quat);
	void multiplicarQuat(const float *q1, const float *q2, float *qout);
	glm::mat4 scale_en_matriz(float scale_tx);
	glm::mat4 translate_en_matriz(float translate_tx, float translate_ty, float translate_tz);
	glm::mat4 rotacion_en_matriz(float rotacion_tx, float rotacion_ty, float rotacion_tz, float rotacion_ta);
};