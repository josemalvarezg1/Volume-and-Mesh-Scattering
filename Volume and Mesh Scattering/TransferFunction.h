#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include "BufferOffset.h"
#include "GLSLProgram.h"
#include "Texture.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

enum structure
{
	POINTC,
	SELECTOR,
	HISTOGRAM,
	POINTSELECT,
};

#define ERROR_T 0.0001
#define HISTDISP_X 0.65
#define HISTDISP_Y 0.65
#define HISTSIZE_X 0.6
#define HISTSIZE_Y 0.6
#define MAPDISP_X 0.65
#define MAPDISP_Y 0.65
#define MAPSIZE_X 0.472
#define MAPSIZE_Y 0.3712
#define POINTDISP_X 0.351
#define POINTDISP_Y 0.23
#define POINTSIZE_X 0.015
#define POINTSIZE_Y 0.015
#define SELECTORDISP_X 0.351
#define SELECTORDISP_Y 0.291
#define SELECTORSIZE_X 0.01
#define SELECTORSIZE_Y 0.1
#define BARDISP_X 0.35
#define BARDISP_Y 0.29
#define BARSIZE_X 0.1
#define BARSIZE_Y 0.1
#define AREADISP_X 0.35
#define AREADISP_Y - 0.12
#define AREASIZE_X 0.6
#define AREASIZE_Y 0.7
#define COLORSDISP_X 0.35
#define COLORSDISP_Y - 0.53
#define COLORSSIZE_X 0.6
#define COLORSSIZE_Y 0.1

class quad_color
{
public:
	std::vector<glm::vec2> coords_vertex;
	std::vector<glm::vec4> colors;
	GLuint vbo, vao;
	quad_color(std::vector<glm::vec2> coords_vertex, std::vector<glm::vec4> colors);
	~quad_color();
	void mobilize(double displ_x, double displ_y);
	void create_vbo();
	void display();
};

class quad_texture
{
public:
	std::vector<glm::vec2> coords_vertex, coords_texture;
	GLuint vbo, vao;
	quad_texture(double size_x, double size_y);
	~quad_texture();
	void create_vbo();
	void display();
};

class region
{
public:
	double displacement_x, displacement_y, size_x, size_y;
	bool vert_movable, horiz_movable;

	region(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable);
	glm::vec2 get_disp();
	glm::vec2 get_min_coords();
	glm::vec2 get_max_coords();
	void mobilize(double displ_x, double displ_y);
	void edit_pos(double displ_x, double displ_y);
	bool intersection(double displ_x, double displ_y);
	bool intersection_x(double displ_x);
	bool intersection_y(double displ_y);
};

class point : public region
{
public:
	double coord_h;
	glm::vec2 coord_sv;

	point(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable, double coord_h, glm::vec2 coord_sv);
	void mobilize_sp(double displ_x, double displ_y);
};

class histogram : public region
{
public:
	std::vector<point*> control_points;
	region *cartesian_map;
	quad_color *connections;

	histogram(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable);
	void mobilize_h(double displ_x, double displ_y);
	int selected(double disp_x, double disp_y);
	bool collision(double disp_x, double disp_y, int index_select);
	bool circles_intersection(glm::vec2 first_circle, glm::vec2 second_circle);
	bool between(double lower_limit, double position, double upper_limit);
	bool creation(double disp_x);
	void update_connections(glm::vec2 coords_xs, glm::vec2 coords_px, glm::vec2 coords_py, glm::vec2 coords_yps);
};

class color_pick
{
public:
	quad_color *color_bar_q, *color_area_q, *color_select_q;
	region *selector, *point, *color_bar_r, *color_area_r;

	color_pick();
	~color_pick();
	void mobilize(double displ_x, double displ_y);
};

class interface_function
{
public:
	histogram *map;
	color_pick *color_picker;
	std::vector<quad_texture*> quads_t;
	std::vector<texture*> textures;
	bool histogram_press, cartesian_map_press, color_bar_press, color_area_press, hide, movable;
	int index_select;
	double x_reference, y_reference;
	CGLSLProgram p1, p2;

	interface_function();
	~interface_function();
	void init_shaders();
	void render_histogram();
	void render_function();
	void render_points();
	void render_color_picker();
	void render_selector_and_point();
	void display();
	void mobilize(double displ_x, double displ_y);
	void update_coords();
	void update_lines();
	void update_color_area();
	void update_color_select();
	bool click_transfer_f(double x, double y, int g_width, int g_height);
	void disable_select();
	bool poscursor_transfer_f(double x, double y, int g_width, int g_height);
	void delete_point();
	std::vector<double> get_color_points();
};