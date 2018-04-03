#include "TransferFunction.h"

double screen_to_world_coord_x(double x, int g_width)
{
	return((((x * 2.0) / double(g_width)) - 1.0));
}

double screen_to_world_coord_y(double y, int g_height)
{
	return((1.0 - ((y * 2.0) / double(g_height))));
}

double world_to_relative_coord(double coord, double min_coord, double max_coord)
{
	return (double((coord - min_coord) / (max_coord - min_coord)));
}

glm::vec3 hsv_to_rgb(double h, double s, double v)
{
	double c, x, m;
	glm::vec3 rgb_aux(0.0);

	c = v * s;
	x = c * (1 - abs(fmod((h / 60.0), 2.0) - 1));
	m = v - c;

	if (0 <= h && h < 60)
		rgb_aux = glm::vec3(c, x, 0.0);
	else
		if (60 <= h && h < 120)
			rgb_aux = glm::vec3(x, c, 0.0);
		else
			if (120 <= h && h < 180)
				rgb_aux = glm::vec3(0.0, c, x);
			else
				if (180 <= h && h < 240)
					rgb_aux = glm::vec3(0.0, x, c);
				else
					if (240 <= h && h < 300)
						rgb_aux = glm::vec3(x, 0.0, c);
					else
						if (300 <= h && h < 360)
							rgb_aux = glm::vec3(c, 0.0, x);
	return rgb_aux + glm::vec3(m);
}

quad_color::quad_color(std::vector<glm::vec2> coords_vertex, std::vector<glm::vec4> colors)
{
	this->coords_vertex = coords_vertex;
	this->colors = colors;
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	this->create_vbo();
}

quad_color::~quad_color()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	this->coords_vertex.clear();
	this->colors.clear();
}

void quad_color::mobilize(double displ_x, double displ_y)
{
	for (size_t i = 0; i < this->coords_vertex.size(); i++)
		this->coords_vertex[i] = glm::vec2(this->coords_vertex[i].x + displ_x, this->coords_vertex[i].y + displ_y);
	create_vbo();
}

void quad_color::create_vbo()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, ((sizeof(glm::vec2) + sizeof(glm::vec4)) * this->coords_vertex.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->coords_vertex.size(), &this->coords_vertex[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->coords_vertex.size(), sizeof(glm::vec4) * this->colors.size(), &this->colors[0]);
	glBindVertexArray(this->vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec2) * this->coords_vertex.size()));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void quad_color::display()
{
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->coords_vertex.size());
	glBindVertexArray(0);
}

quad_texture::quad_texture(double size_x, double size_y)
{
	glm::vec2 max_coords, min_coords;
	max_coords = glm::vec2((size_x / 2.0), (size_y / 2.0));
	min_coords = glm::vec2((-size_x / 2.0), (-size_y / 2.0f));
	this->coords_vertex.push_back(glm::vec2(min_coords.x, max_coords.y));
	this->coords_vertex.push_back(glm::vec2(max_coords.x, max_coords.y));
	this->coords_vertex.push_back(glm::vec2(min_coords.x, min_coords.y));
	this->coords_vertex.push_back(glm::vec2(max_coords.x, min_coords.y));
	this->coords_texture.push_back(glm::vec2(0.0f, 0.0f));
	this->coords_texture.push_back(glm::vec2(1.0f, 0.0f));
	this->coords_texture.push_back(glm::vec2(0.0f, 1.0f));
	this->coords_texture.push_back(glm::vec2(1.0f, 1.0f));
	this->create_vbo();
}

quad_texture::~quad_texture()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	this->coords_vertex.clear();
	this->coords_texture.clear();
}

void quad_texture::create_vbo()
{
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * (this->coords_vertex.size() + this->coords_texture.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->coords_vertex.size(), &this->coords_vertex[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->coords_vertex.size(), sizeof(glm::vec2) * this->coords_texture.size(), &this->coords_texture[0]);
	glBindVertexArray(this->vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec2) * this->coords_vertex.size()));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void quad_texture::display()
{
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->coords_vertex.size());
	glBindVertexArray(0);
}


region::region(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable)
{
	this->displacement_x = disp_x;
	this->displacement_y = disp_y;
	this->size_x = size_x;
	this->size_y = size_y;
	this->vert_movable = vert_movable;
	this->horiz_movable = horiz_movable;
}

glm::vec2 region::get_disp()
{
	return glm::vec2(displacement_x, displacement_y);
}

glm::vec2 region::get_min_coords()
{
	return glm::vec2(displacement_x - (size_x / 2.0), displacement_y - (size_y / 2.0));
}

glm::vec2 region::get_max_coords()
{
	return glm::vec2(displacement_x + (size_x / 2.0), displacement_y + (size_y / 2.0));
}

void region::mobilize(double displ_x, double displ_y)
{
	this->displacement_x = (this->displacement_x + displ_x);
	this->displacement_y = (this->displacement_y + displ_y);
}

void region::edit_pos(double displ_x, double displ_y)
{
	if (this->horiz_movable) this->displacement_x = (this->displacement_x + displ_x);
	if (this->vert_movable) this->displacement_y = (this->displacement_y + displ_y);
}

bool region::intersection(double displ_x, double displ_y)
{
	if (this->intersection_x(displ_x) && this->intersection_y(displ_y)) return true;
	return false;
}

bool region::intersection_x(double displ_x)
{
	if (((this->displacement_x - (this->size_x / 2.0)) < displ_x) && (displ_x < (this->displacement_x + (this->size_x / 2.0)))) return true;
	return false;
}

bool region::intersection_y(double displ_y)
{
	if (((this->displacement_y - (this->size_y / 2.0)) < displ_y) && (displ_y < (this->displacement_y + (this->size_y / 2.0)))) return true;
	return false;
}

point::point(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable, double coord_h, glm::vec2 coord_sv) : region(disp_x, disp_y, size_x, size_y, horiz_movable, vert_movable)
{
	this->coord_h = coord_h;
	this->coord_sv = coord_sv;
}

void point::mobilize_sp(double displ_x, double displ_y)
{
	this->coord_h = (this->coord_h + displ_x);
	this->coord_sv = this->coord_sv + glm::vec2(displ_x, displ_y);
}

histogram::histogram(double disp_x, double disp_y, double size_x, double size_y, bool horiz_movable, bool vert_movable) : region(disp_x, disp_y, size_x, size_y, horiz_movable, vert_movable)
{
	point *startPoint, *finalPoint;
	std::vector<glm::vec2> coords_vertex;
	std::vector<glm::vec4> colors;
	this->cartesian_map = new region(MAPDISP_X, MAPDISP_Y, MAPSIZE_X, MAPSIZE_Y, true, true);
	startPoint = new point((disp_x - (this->cartesian_map->size_x / 2.0) + ERROR), (disp_y - (this->cartesian_map->size_y / 2.0) + ERROR), POINTSIZE_X, POINTSIZE_Y, false, true, SELECTORDISP_X, glm::vec2(POINTDISP_X, AREADISP_Y - (AREASIZE_Y / 2.0)));
	finalPoint = new point((disp_x + (this->cartesian_map->size_x / 2.0) - ERROR), (disp_y + (this->cartesian_map->size_y / 2.0) - ERROR), POINTSIZE_X, POINTSIZE_Y, false, true, SELECTORDISP_X, glm::vec2(POINTDISP_X, POINTDISP_Y));
	this->control_points.push_back(startPoint);
	this->control_points.push_back(finalPoint);
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	coords_vertex.push_back(glm::vec2(this->control_points[0]->displacement_x, this->control_points[0]->displacement_y));
	coords_vertex.push_back(glm::vec2(this->control_points[0]->displacement_x, this->control_points[0]->displacement_y));
	coords_vertex.push_back(glm::vec2(this->control_points[1]->displacement_x, this->control_points[1]->displacement_y));
	coords_vertex.push_back(glm::vec2(this->control_points[1]->displacement_x, this->control_points[0]->displacement_y));
	this->connections = new quad_color(coords_vertex, colors);
}

void histogram::mobilize_h(double displ_x, double displ_y)
{
	this->mobilize(displ_x, displ_y);
	this->cartesian_map->mobilize(displ_x, displ_y);
	for (unsigned int i = 0; i < this->control_points.size(); i++)
	{
		this->control_points[i]->mobilize(displ_x, displ_y);
		this->control_points[i]->mobilize_sp(displ_x, displ_y);
	}
	this->connections->mobilize(displ_x, displ_y);
}

int histogram::selected(double disp_x, double disp_y)
{
	for (unsigned int i = 0; i < this->control_points.size(); ++i) if (this->control_points[i]->intersection(disp_x, disp_y)) return i;
	return -1;
}

bool histogram::collision(double disp_x, double disp_y, int index_select)
{
	glm::vec2 disp1, disp2;
	double lower_limit_x, upper_limit_x;
	disp1 = this->control_points[index_select]->get_disp() + glm::vec2(disp_x, disp_y);
	lower_limit_x = this->control_points[index_select]->displacement_x;
	upper_limit_x = disp1.x;
	for (unsigned int i = 0; i < this->control_points.size(); ++i)
	{
		if (i != index_select)
		{
			disp2 = this->control_points[i]->get_disp();
			if ((this->circles_intersection(disp1, disp2)) || (this->between(lower_limit_x + (POINTSIZE_X / 2.0), disp2.x, upper_limit_x + (POINTSIZE_X / 2.0))) || (this->between(upper_limit_x - (POINTSIZE_X / 2.0), disp2.x, lower_limit_x - (POINTSIZE_X / 2.0))))
				return true;
		}
	}
	return false;
}

bool histogram::circles_intersection(glm::vec2 first_circle, glm::vec2 second_circle)
{
	if ((pow((second_circle.x - first_circle.x), 2) + pow((first_circle.y - second_circle.y), 2)) <= (pow((POINTSIZE_X), 2))) return true;
	return false;
}

bool histogram::between(double lower_limit, double position, double upper_limit)
{
	if ((lower_limit < position) && (upper_limit > position)) return true;
	return false;
}

bool histogram::creation(double disp_x)
{
	for (unsigned int i = 0; i < this->control_points.size(); i++) if (this->control_points[i]->intersection_x(disp_x)) return true;
	return false;
}

void histogram::update_connections(glm::vec2 coords_xs, glm::vec2 coords_px, glm::vec2 coords_py, glm::vec2 coords_yps)
{
	double h, s, v, alpha;
	glm::vec3 color;
	this->connections->coords_vertex.clear();
	this->connections->colors.clear();
	for (unsigned int i = 0; i < this->control_points.size(); i++)
	{
		h = world_to_relative_coord(this->control_points[i]->coord_h, coords_xs[0], coords_xs[1]) * 360.0;
		s = world_to_relative_coord(this->control_points[i]->coord_sv.x, coords_px[0], coords_px[1]);
		v = world_to_relative_coord(this->control_points[i]->coord_sv.y, coords_py[0], coords_py[1]);
		alpha = world_to_relative_coord(this->control_points[i]->displacement_y, coords_yps[0], coords_yps[1]);
		color = hsv_to_rgb(h, s, v);
		this->connections->colors.push_back(glm::vec4(color, alpha));
		this->connections->colors.push_back(glm::vec4(color, 0.0));
		this->connections->coords_vertex.push_back(glm::vec2(this->control_points[i]->displacement_x, this->control_points[i]->displacement_y));
		this->connections->coords_vertex.push_back(glm::vec2(this->control_points[i]->displacement_x, this->displacement_y - (this->cartesian_map->size_y / 2.0)));
	}
	this->connections->create_vbo();
}

color_pick::color_pick()
{
	std::vector<glm::vec2> coords_vertex;
	std::vector<glm::vec4> colors;
	double i;
	for (i = 0; i <= (BARSIZE_X * 6); i += BARSIZE_X)
	{
		coords_vertex.push_back(glm::vec2(BARDISP_X + i, BARDISP_Y + (BARSIZE_Y / 2.0)));
		coords_vertex.push_back(glm::vec2(BARDISP_X + i, BARDISP_Y - (BARSIZE_Y / 2.0)));
	}
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 1.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 1.0, 0.0, 1.0));
	colors.push_back(glm::vec4(0.0, 1.0, 0.0, 1.0));
	colors.push_back(glm::vec4(0.0, 1.0, 0.0, 1.0));
	colors.push_back(glm::vec4(0.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(0.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 1.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 1.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 1.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 1.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	this->color_bar_q = new quad_color(coords_vertex, colors);
	coords_vertex.clear();
	colors.clear();
	for (i = 0; i <= AREASIZE_X; i += AREASIZE_X)
	{
		coords_vertex.push_back(glm::vec2(AREADISP_X + i, AREADISP_Y + (AREASIZE_Y / 2.0)));
		coords_vertex.push_back(glm::vec2(AREADISP_X + i, AREADISP_Y - (AREASIZE_Y / 2.0)));
	}
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 0.0, 1.0));
	this->color_area_q = new quad_color(coords_vertex, colors);
	coords_vertex.clear();
	colors.clear();
	for (i = 0; i <= COLORSSIZE_X; i += COLORSSIZE_X)
	{
		coords_vertex.push_back(glm::vec2(COLORSDISP_X + i, COLORSDISP_Y + (COLORSSIZE_Y / 2.0)));
		coords_vertex.push_back(glm::vec2(COLORSDISP_X + i, COLORSDISP_Y - (COLORSSIZE_Y / 2.0)));
		colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
		colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	}
	this->color_select_q = new quad_color(coords_vertex, colors);
	this->selector = new region(SELECTORDISP_X, SELECTORDISP_Y, SELECTORSIZE_X, SELECTORSIZE_Y, true, false);
	this->point = new region(POINTDISP_X, POINTDISP_Y, POINTSIZE_X, POINTSIZE_Y, true, true);
	this->color_bar_r = new region((BARDISP_X + (BARSIZE_X * 3)), BARDISP_Y, (BARSIZE_X * 6), BARSIZE_Y, false, false);
	this->color_area_r = new region((COLORSDISP_X + AREASIZE_X / 2.0), AREADISP_Y, AREASIZE_X, AREASIZE_Y, false, false);
}

color_pick::~color_pick()
{
	this->color_bar_q->~quad_color();
	this->color_area_q->~quad_color();
	this->color_select_q->~quad_color();
}

void color_pick::mobilize(double displ_x, double displ_y)
{
	this->color_bar_q->mobilize(displ_x, displ_y);
	this->color_area_q->mobilize(displ_x, displ_y);
	this->color_select_q->mobilize(displ_x, displ_y);
	this->selector->mobilize(displ_x, displ_y);
	this->point->mobilize(displ_x, displ_y);
	this->color_bar_r->mobilize(displ_x, displ_y);
	this->color_area_r->mobilize(displ_x, displ_y);
}


interface_function::interface_function()
{
	quad_texture *point_quad, *selector_quad, *histogram_quad;
	texture *point_text, *selector_text, *histogram_text, *point_select_text;
	point_text = new texture("../img/point.png");
	selector_text = new texture("../img/selector.png");
	histogram_text = new texture("../img/histogram.png");
	point_select_text = new texture("../img/pointSelect.png");
	point_quad = new quad_texture(POINTSIZE_X, POINTSIZE_Y);
	selector_quad = new quad_texture(SELECTORSIZE_X, SELECTORSIZE_Y);
	histogram_quad = new quad_texture(HISTSIZE_X, HISTSIZE_Y);
	this->map = new histogram(HISTDISP_X, HISTDISP_Y, HISTSIZE_X, HISTSIZE_Y, false, false);
	this->color_picker = new color_pick();
	this->quads_t.push_back(point_quad);
	this->quads_t.push_back(selector_quad);
	this->quads_t.push_back(histogram_quad);
	this->textures.push_back(point_text);
	this->textures.push_back(selector_text);
	this->textures.push_back(histogram_text);
	this->textures.push_back(point_select_text);
	this->histogram_press = false;
	this->cartesian_map_press = false;
	this->color_bar_press = false;
	this->color_area_press = false;
	this->hide = false;
	this->movable = false;
	this->index_select = -1;
	this->init_shaders();
}

interface_function::~interface_function()
{
	this->quads_t.clear();
	this->textures.clear();
}

void interface_function::init_shaders()
{
	this->p1.loadShader("Shaders/quad.vert", CGLSLProgram::VERTEX);
	this->p1.loadShader("Shaders/quad.frag", CGLSLProgram::FRAGMENT);
	this->p2.loadShader("Shaders/function.vert", CGLSLProgram::VERTEX);
	this->p2.loadShader("Shaders/function.frag", CGLSLProgram::FRAGMENT);

	this->p1.create_link();
	this->p2.create_link();

	this->p1.enable();
	this->p1.addAttribute("vert_coords");
	this->p1.addAttribute("tex_coords");
	this->p1.addUniform("texture_id");
	this->p1.addUniform("displacement");
	this->p1.disable();

	this->p2.enable();
	this->p2.addAttribute("vert_coords");
	this->p2.addAttribute("color");
	this->p2.disable();
}

void interface_function::render_histogram()
{
	glBindTexture(GL_TEXTURE_2D, this->textures[HISTOGRAM]->get_texture_ID());
	this->p1.enable();
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->map->displacement_x, (GLfloat) this->map->displacement_y);
	glUniform1i(this->p1.getLocation("texture_id"), 0);
	this->quads_t[HISTOGRAM]->display();
	this->p1.disable();
}

void interface_function::render_function()
{
	this->p2.enable();
	this->map->connections->display();
	this->p2.disable();
}

void interface_function::render_points()
{
	this->p1.enable();
	for (unsigned int i = 0; i < this->map->control_points.size(); i++)
	{
		if (this->index_select == i)
			glBindTexture(GL_TEXTURE_2D, this->textures[POINTSELECT]->get_texture_ID());
		else
			glBindTexture(GL_TEXTURE_2D, this->textures[POINTC]->get_texture_ID());
		glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->map->control_points[i]->displacement_x, (GLfloat) this->map->control_points[i]->displacement_y);
		this->quads_t[POINTC]->display();
	}
	this->p1.disable();
}

void interface_function::render_color_picker()
{
	this->p2.enable();
	this->color_picker->color_bar_q->display();
	this->color_picker->color_area_q->display();
	this->color_picker->color_select_q->display();
	this->p2.disable();
}

void interface_function::render_selector_and_point()
{
	this->p1.enable();
	glBindTexture(GL_TEXTURE_2D, this->textures[SELECTOR]->get_texture_ID());
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->color_picker->selector->displacement_x, (GLfloat) this->color_picker->selector->displacement_y);
	glUniform1i(this->p1.getLocation("textureId"), 0);
	this->quads_t[SELECTOR]->display();
	glBindTexture(GL_TEXTURE_2D, this->textures[POINTC]->get_texture_ID());
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->color_picker->point->displacement_x, (GLfloat) this->color_picker->point->displacement_y);
	this->quads_t[POINTC]->display();
	this->p1.disable();
}

void interface_function::display()
{
	if (!hide)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		render_histogram();
		render_function();
		render_points();
		render_color_picker();
		render_selector_and_point();

		glDisable(GL_BLEND);
	}
}

void interface_function::mobilize(double displ_x, double displ_y)
{
	this->map->mobilize_h(displ_x, displ_y);
	this->color_picker->mobilize(displ_x, displ_y);
}

void interface_function::update_coords()
{
	if (this->index_select != -1 && !this->color_bar_press && !this->color_area_press)
	{
		if (this->color_picker->point->displacement_x != this->map->control_points[this->index_select]->coord_sv.x || this->color_picker->point->displacement_y != this->map->control_points[this->index_select]->coord_sv.y || this->color_picker->selector->displacement_x != this->map->control_points[this->index_select]->coord_h)
		{
			this->color_picker->selector->displacement_x = this->map->control_points[this->index_select]->coord_h;
			this->color_picker->point->displacement_x = this->map->control_points[this->index_select]->coord_sv.x;
			this->color_picker->point->displacement_y = this->map->control_points[this->index_select]->coord_sv.y;
			this->update_color_area();
			this->update_color_select();
		}
	}
}

void interface_function::update_lines()
{
	glm::vec2 coords_xs, coords_xp, coords_yp, coords_yps;
	coords_xs = glm::vec2(this->color_picker->color_bar_r->displacement_x - (this->color_picker->color_bar_r->size_x / 2.0), this->color_picker->color_bar_r->displacement_x + (this->color_picker->color_bar_r->size_x / 2.0));
	coords_xp = glm::vec2(this->color_picker->color_area_r->displacement_x - (this->color_picker->color_area_r->size_x / 2.0), this->color_picker->color_area_r->displacement_x + (this->color_picker->color_area_r->size_x / 2.0));
	coords_yp = glm::vec2(this->color_picker->color_area_r->displacement_y - (this->color_picker->color_area_r->size_y / 2.0), this->color_picker->color_area_r->displacement_y + (this->color_picker->color_area_r->size_y / 2.0));
	coords_yps = glm::vec2(this->map->cartesian_map->displacement_y - (this->map->cartesian_map->size_y / 2.0), this->map->cartesian_map->displacement_y + (this->map->cartesian_map->size_y / 2.0));
	this->map->update_connections(coords_xs, coords_xp, coords_yp, coords_yps);
}

void interface_function::update_color_area()
{
	glm::vec3 color;
	double h;
	h = world_to_relative_coord(this->color_picker->selector->displacement_x, this->color_picker->color_bar_r->displacement_x - (this->color_picker->color_bar_r->size_x / 2.0), this->color_picker->color_bar_r->displacement_x + (this->color_picker->color_bar_r->size_x / 2.0)) * 360.0;
	color = hsv_to_rgb(h, 1.0, 1.0);
	this->color_picker->color_area_q->colors[2] = glm::vec4(color, 1.0);
	this->color_picker->color_area_q->create_vbo();
}

void interface_function::update_color_select()
{
	double h, s, v;
	glm::vec3 color;
	h = world_to_relative_coord(this->color_picker->selector->displacement_x, this->color_picker->color_bar_r->displacement_x - (this->color_picker->color_bar_r->size_x / 2.0), this->color_picker->color_bar_r->displacement_x + (this->color_picker->color_bar_r->size_x / 2.0)) * 360.0;
	s = world_to_relative_coord(this->color_picker->point->displacement_x, this->color_picker->color_area_r->displacement_x - (this->color_picker->color_area_r->size_x / 2.0), this->color_picker->color_area_r->displacement_x + (this->color_picker->color_area_r->size_x / 2.0));
	v = world_to_relative_coord(this->color_picker->point->displacement_y, this->color_picker->color_area_r->displacement_y - (this->color_picker->color_area_r->size_x / 2.0), this->color_picker->color_area_r->displacement_y + (this->color_picker->color_area_r->size_x / 2.0));
	color = hsv_to_rgb(h, s, v);
	for (unsigned int i = 0; i < this->color_picker->color_select_q->colors.size(); i++) this->color_picker->color_select_q->colors[i] = glm::vec4(color, 1.0);
	this->color_picker->color_select_q->create_vbo();
}

bool interface_function::click_transfer_f(double x, double y, int g_width, int g_height)
{
	if (!hide)
	{
		double coord_x, coord_y;
		if (!histogram_press && !cartesian_map_press && !color_bar_press && !color_area_press)
		{
			coord_x = screen_to_world_coord_x(x, g_width);
			coord_y = screen_to_world_coord_y(y, g_height);

			if (this->map->intersection(coord_x, coord_y))
			{
				if (this->map->cartesian_map->intersection(coord_x, coord_y))
				{
					this->index_select = this->map->selected(coord_x, coord_y);
					if (this->index_select == -1 && !this->map->creation(coord_x))
					{
						point *point_new;
						point_new = new point(coord_x, coord_y, POINTSIZE_X, POINTSIZE_Y, true, true, this->color_picker->selector->displacement_x, this->color_picker->point->get_disp());
						this->map->control_points.push_back(point_new);
						std::sort(this->map->control_points.begin(), this->map->control_points.end(), [](const point *lhs, const point *rhs) { return lhs->displacement_x < rhs->displacement_x; });
						index_select = this->map->selected(coord_x, coord_y);
						update_lines();
					}
					if (this->index_select != -1)
					{
						this->x_reference = coord_x;
						this->y_reference = coord_y;
						this->update_color_area();
						this->update_color_select();
						this->cartesian_map_press = true;
					}
				}
				else
				{
					this->x_reference = coord_x;
					this->y_reference = coord_y;
					this->index_select = -1;
					histogram_press = true;
				}
				return true;
			}
			else
			{
				if (this->color_picker->color_bar_r->intersection(coord_x, coord_y))
				{
					if (!this->color_picker->selector->intersection(coord_x, coord_y))
					{
						this->color_picker->selector->displacement_x = coord_x;
						if (this->index_select != -1)
						{
							this->map->control_points[index_select]->coord_h = this->color_picker->selector->displacement_x;
							this->update_lines();
						}
						this->update_color_area();
						this->update_color_select();
					}
					this->x_reference = coord_x;
					this->y_reference = coord_y;
					this->color_bar_press = true;
					return true;
				}
				else
				{
					if (this->color_picker->color_area_r->intersection(coord_x, coord_y))
					{
						if (!this->color_picker->point->intersection(coord_x, coord_y))
						{
							this->color_picker->point->displacement_x = coord_x;
							this->color_picker->point->displacement_y = coord_y;
							if (this->index_select != -1)
							{
								this->map->control_points[index_select]->coord_sv.x = (float) this->color_picker->point->displacement_x;
								this->map->control_points[index_select]->coord_sv.y = (float) this->color_picker->point->displacement_y;
								this->update_lines();
							}
							this->update_color_select();
						}
						this->x_reference = coord_x;
						this->y_reference = coord_y;
						this->color_area_press = true;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void interface_function::disable_select()
{
	this->histogram_press = false;
	this->cartesian_map_press = false;
	this->color_bar_press = false;
	this->color_area_press = false;
}

bool interface_function::poscursor_transfer_f(double x, double y, int g_width, int g_height)
{
	if (!hide)
	{
		double disp_x, disp_y;

		disp_x = screen_to_world_coord_x(x, g_width);
		disp_y = screen_to_world_coord_y(y, g_height);

		if (this->histogram_press)
		{
			if (this->movable)
				this->mobilize((disp_x - this->x_reference), (disp_y - this->y_reference));
			this->x_reference = disp_x;
			this->y_reference = disp_y;
			return true;
		}

		if (this->cartesian_map_press)
		{
			if (!this->map->collision((disp_x - this->x_reference), (disp_y - this->y_reference), this->index_select) && this->map->cartesian_map->intersection(this->map->control_points[index_select]->displacement_x + (disp_x - this->x_reference), this->map->control_points[index_select]->displacement_y + (disp_y - this->y_reference)))
			{
				this->map->control_points[index_select]->edit_pos((disp_x - this->x_reference), (disp_y - this->y_reference));
				this->map->control_points[index_select]->coord_h = this->color_picker->selector->displacement_x;
				this->map->control_points[index_select]->coord_sv = this->color_picker->point->get_disp();
				this->update_lines();
			}
			this->x_reference = disp_x;
			this->y_reference = disp_y;
			return true;
		}

		if (this->color_bar_press)
		{
			if (this->color_picker->color_bar_r->intersection_x(this->color_picker->selector->displacement_x + (disp_x - this->x_reference)))
			{
				this->color_picker->selector->edit_pos((disp_x - this->x_reference), (disp_y - this->y_reference));
				if (this->index_select != -1)
				{
					this->map->control_points[index_select]->coord_h = this->color_picker->selector->displacement_x;
					this->update_lines();
				}
				this->update_color_area();
				this->update_color_select();
			}
			this->x_reference = disp_x;
			this->y_reference = disp_y;
			return true;
		}

		if (this->color_area_press)
		{
			if (this->color_picker->color_area_r->intersection(this->color_picker->point->displacement_x + (disp_x - this->x_reference), this->color_picker->point->displacement_y + (disp_y - this->y_reference)))
			{
				this->color_picker->point->edit_pos((disp_x - this->x_reference), (disp_y - this->y_reference));
				if (this->index_select != -1)
				{
					this->map->control_points[index_select]->coord_sv.x = (float) this->color_picker->point->displacement_x;
					this->map->control_points[index_select]->coord_sv.y = (float) this->color_picker->point->displacement_y;
					this->update_lines();
				}
				this->update_color_select();
			}
			this->x_reference = disp_x;
			this->y_reference = disp_y;
			return true;
		}
	}
	return false;
}

void interface_function::delete_point()
{
	if (this->index_select > 0 && this->index_select != this->map->control_points.size() - 1)
	{
		this->map->control_points.erase(this->map->control_points.begin() + (this->index_select));
		this->index_select = -1;
		this->update_lines();
	}
}

std::vector<double> interface_function::get_color_points()
{
	std::vector<double> points;
	double valueX, h, s, v, alpha;
	glm::vec3 rgb;

	for (unsigned int i = 0; i < this->map->control_points.size(); ++i)
	{
		valueX = world_to_relative_coord(this->map->control_points[i]->displacement_x, this->map->cartesian_map->displacement_x - (this->map->cartesian_map->size_x / 2.0), this->map->cartesian_map->displacement_x + (this->map->cartesian_map->size_x / 2.0));
		h = world_to_relative_coord(this->map->control_points[i]->coord_h, this->color_picker->color_bar_r->displacement_x - (this->color_picker->color_bar_r->size_x / 2.0), this->color_picker->color_bar_r->displacement_x + (this->color_picker->color_bar_r->size_x / 2.0)) * 360.0;
		s = world_to_relative_coord(this->map->control_points[i]->coord_sv.x, this->color_picker->color_area_r->displacement_x - (this->color_picker->color_area_r->size_x / 2.0), this->color_picker->color_area_r->displacement_x + (this->color_picker->color_area_r->size_x / 2.0));
		v = world_to_relative_coord(this->map->control_points[i]->coord_sv.y, this->color_picker->color_area_r->displacement_y - (this->color_picker->color_area_r->size_y / 2.0), this->color_picker->color_area_r->displacement_y + (this->color_picker->color_area_r->size_y / 2.0));
		alpha = world_to_relative_coord(this->map->control_points[i]->displacement_y, this->map->cartesian_map->displacement_y - (this->map->cartesian_map->size_y / 2.0), this->map->cartesian_map->displacement_y + (this->map->cartesian_map->size_y / 2.0));
		rgb = hsv_to_rgb(h, s, v);
		points.push_back(valueX);
		points.push_back(rgb.r);
		points.push_back(rgb.g);
		points.push_back(rgb.b);
		points.push_back(alpha);
	}
	return points;
}