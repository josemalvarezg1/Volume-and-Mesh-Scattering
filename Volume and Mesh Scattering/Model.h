#pragma once
#define BUFFER_OFFSET(offset) ((char*)NULL + (offset))
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
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
#include "InterfaceModel.h"
#include "G-Buffer.h"
#include "Material.h"
#include "Halton.h"
#include "ScatteredMap.h"
#include "InterfaceMenu.h"
#include "GLSLProgram.h"

class mesh
{
public:
	GLuint vbo, vao;
	glm::quat rotation;
	GLboolean change_values;
	GLfloat scale, max_value, asymmetry_param_g, refractive_index, q, radius, epsilon, gamma, bias;
	std::vector<glm::vec3> vertices, normals, bounding_box;
	glm::vec3 max_vertex, min_vertex, center, translation, ambient_comp, diffuse_comp, specular_comp;
	material_m current_material;
	interface_model *model_interface;
	bool visible_interface;

	mesh();
	~mesh();
	void set_max_min_value(GLfloat x, GLfloat y, GLfloat z);
	std::vector<std::string> process_attribute(std::string value);
	std::vector<std::string> split(std::string s, char delim);
	void calculate_center();
	void calculate_normals(std::vector<glm::vec3> &aux_normals, std::vector<glm::vec3> aux_vertices, std::vector<glm::uvec3> index_vertices);
	void load(std::string path);
	void create_vbo();
	void click_model();
	void not_click_model();
	void update_interface();
	bool no_collision(glm::mat4 &model);
};

class model {
public:
	CGLSLProgram glsl_g_buffer, glsl_g_buffer_plane, glsl_scattered_map, glsl_blending, glsl_phong;
	mesh *scene_model;
	halton *halton_generator;
	light_buffer *light_buffers;
	materials_set *materials;
	scattered_map *scattered_maps;
	GLuint num_of_ortho_cameras, num_of_samples_per_frag, selected_camera, quad_vao, quad_vbo;

	model(int g_width, int g_height);
	~model();
	void init_shaders();
	void display(glm::mat4 projection, glm::mat4 view, int g_width, int g_height, float shininess, bool scattering_model, bool change_light, bool model_center, bool specular_flag, glm::vec3 light_translation, glm::vec3 diffuse_comp, glm::vec3 ambient_comp, glm::vec3 specular_comp, glm::vec3 camera_position);
	void display_g_buffer(texture_t current_texture_type);
	void render_quad();
};

