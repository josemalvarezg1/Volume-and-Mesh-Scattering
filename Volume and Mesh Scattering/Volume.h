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
	GLuint width, height, depth, bits, volume_text;
	glm::quat rotation;
	glm::vec3 translation, scattering_coeff, extinction_coeff;
	GLfloat escalation, step, asymmetry_param_g, radius;
	glm::vec4 back_radiance;
	bool change_values;

	volume(std::string path, GLuint width, GLuint height, GLuint depth, GLuint bits);
	~volume();
};

class volume_render
{
public:
	CGLSLProgram backface, raycasting;
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
	void drop_path(int count, const char** paths);
	bool process_path(std::string path);
	glm::uvec4 get_parameters(std::string path);
	void init_shaders();
	void scroll_volume(double y_offset);
	bool click_volume(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position, bool type);
	bool pos_cursor_volume(double x, double y);
	void disable_select();
	bool intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position);
	void load_transfer_func_t(GLfloat data[][4]);
	void update_transfer_function(std::vector<double> points);
	void create_backface_text();
	bool create_frame_buffer();
	void render_cube(glm::mat4 &MVP);
	void render_cube_raycast(glm::mat4 &MVP, glm::mat4 &model, glm::vec3 view_pos, std::vector<light*> scene_lights);
	void display(glm::mat4 &viewProjection, glm::vec3 view_pos, std::vector<light*> scene_lights);
	void change_volume(int type);
	void resize_screen(const glm::vec2 screen);

	void update_interface();
};