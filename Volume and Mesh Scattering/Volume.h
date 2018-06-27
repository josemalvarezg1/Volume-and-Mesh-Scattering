#pragma once
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "GLSLProgram.h"
#include "Light.h"
#include "InterfaceVolume.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct face
{
	float value;
	unsigned int index;
};

class cube
{
public:
	GLuint vbo_cube, vao_cube, ebo_cube;
	cube();
	~cube();
	void display();
};

class volume
{
public:
	std::string name;
	GLuint width, height, depth, bits, volume_text, light_volume_text, volume_buffer[2], render_texture, previous_texture, texture_vao, texture_vbo;
	std::vector<GLuint> current_index;
	glm::quat rotation;
	glm::vec3 translation;
	GLfloat escalation, step, cos_beta, cos_gamma;
	std::vector<GLfloat> step_light_volume;
	std::vector<glm::vec3> bounding_box;
	glm::vec4 back_radiance;
	bool change_values;

	volume(std::string path, GLuint width, GLuint height, GLuint depth, GLuint bits, GLuint g_width, GLuint g_height);
	void create_quad_light_volume();
	void update_light_volume_textures(GLuint g_width, GLuint g_height);
	~volume();
};

class volume_render
{
public:
	CGLSLProgram backface, raycasting, lightcube;
	int g_width, g_height;
	int index_select;
	GLuint frame_buffer, backface_text, transfer_function_text;
	cube *unitary_cube;
	std::vector<volume*> volumes;
	bool press_volume_right, press_volume_left;
	double x_reference, y_reference;
	interface_volume *volume_interface;
	bool visible_interface;	

	volume_render(int g_width, int g_height);
	~volume_render();
	void drop_path(int count, const char** paths, GLuint g_width, GLuint g_height);
	bool process_path(std::string path);
	glm::uvec4 get_parameters(std::string path);
	void init_shaders();
	void scroll_volume(double y_offset);
	bool click_volume(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position, bool type);
	bool pos_cursor_volume(double x, double y);
	void disable_select();
	std::vector<glm::vec4> calculate_dir_max(glm::vec3 light_pos, glm::mat4 model);
	unsigned int get_axis(unsigned value);
	glm::vec4 get_position(int index);
	bool intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position);
	void load_transfer_func_t(GLfloat data[][4]);
	void update_transfer_function(std::vector<double> points);
	void create_backface_text();
	bool create_frame_buffer();
	void render_cube(glm::mat4 &MVP);
	void render_light_cube(glm::mat4 &projection, glm::mat4 &model, glm::vec3 view_pos, light* scene_lights, glm::mat4 view);
	void render_cube_raycast(glm::mat4 &MVP, glm::mat4 &model, glm::vec3 view_pos, light* scene_lights, glm::mat4 view_projection);
	void display(glm::mat4 &projection, glm::mat4 &view, glm::vec3 view_pos, light* scene_lights);
	void change_volume(int type);
	void resize_screen(const glm::vec2 screen);
	void update_interface();
	bool no_collision(glm::mat4 &model);
};