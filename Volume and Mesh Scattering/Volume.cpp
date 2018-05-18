#include "Volume.h"

std::vector<std::string> split(const std::string &s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (getline(ss, item, delim))
		tokens.push_back(item);
	return tokens;
}

cube::cube()
{
	static GLfloat vertex_cube[] =
	{
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 0.0f,
		-0.5f, 0.5f,0.5f,		0.0f, 1.0f, 0.0f,
		0.5f, 0.5f,-0.5f,		1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f,-0.5f,		0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,0.5f,		0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,-0.5f,		0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f
	};

	static GLuint elements[] =
	{
		3, 2, 6, 7, 4, 2, 0,
		3, 1, 6, 5, 4, 1, 0
	};

	glGenVertexArrays(1, &this->vao_cube);
	glBindVertexArray(this->vao_cube);

	glGenBuffers(1, &this->vbo_cube);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_cube);
	glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat), vertex_cube, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(sizeof(GL_FLOAT) * 3));

	glGenBuffers(1, &this->ebo_cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo_cube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 14 * sizeof(GLuint), elements, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

cube::~cube()
{
	glDeleteVertexArrays(1, &this->vao_cube);
	glDeleteBuffers(1, &this->vbo_cube);
	glDeleteBuffers(1, &this->ebo_cube);
}

void cube::display()
{
	glBindVertexArray(this->vao_cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo_cube);
	glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

volume::volume(std::string path, GLuint width, GLuint height, GLuint depth, GLuint bits)
{
	std::ifstream file(path, std::ios::binary);
	size_t length;
	if (!file.is_open())
		return;
	file.seekg(0, file.end);
	length = static_cast<size_t>(file.tellg());
	this->name = path;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->bits = bits;
	this->change_values = true;
	this->current_index = -1;
	this->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->translation = glm::vec3(0.0f);
	this->escalation = 4.0f;
	this->asymmetry_param_g = 0.77f;
	this->radius = 0.5f;
	this->scattering_coeff = glm::vec3(0.02894f, 0.02145f, 0.01215f);
	this->extinction_coeff = glm::vec3(0.0002f, 0.0028f, 0.016f);
	this->back_radiance = glm::vec4(1.0f);
	this->step = (GLfloat)(1.0f / sqrt((this->width * this->width) + (this->height * this->height) + (this->depth * this->depth)));
	this->step_light_volume = -1.0f;
	if (this->bits == 8u)
		if (length == this->width * this->height * this->depth)
		{
			char *texture_data;
			texture_data = new char[length];
			file.seekg(0, file.beg);
			file.read((char*)texture_data, length);
			file.close();
			glGenTextures(1, &this->volume_text);
			glBindTexture(GL_TEXTURE_3D, this->volume_text);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, this->width, this->height, this->depth, 0, GL_RED, GL_UNSIGNED_BYTE, texture_data);
			glBindTexture(GL_TEXTURE_3D, 0);

			GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			
			glGenVertexArrays(1, &texture_vao);
			glGenBuffers(1, &texture_vbo);
			glGenFramebuffers(1, &this->volume_buffer);
			this->create_quad_light_volume();

			glGenTextures(1, &this->render_texture);
			glGenTextures(1, &this->previous_texture);

			glBindFramebuffer(GL_FRAMEBUFFER, this->volume_buffer);
			
			glBindTexture(GL_TEXTURE_2D, this->render_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->render_texture, 0);
			
			glBindTexture(GL_TEXTURE_2D, this->previous_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->previous_texture, 0);
			
			glDrawBuffers(2, attachments);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			delete[] texture_data;
		}
		else
		{
			std::cout << "Error: dimensiones erróneas" << std::endl;
			return;
		}
	else
		if (length == this->width * this->height * this->depth * 2u)
		{
			short *texture_data;
			texture_data = new short[length];
			file.seekg(0, file.beg);
			file.read((char*)texture_data, length);
			file.close();
			glGenTextures(1, &this->volume_text);
			glBindTexture(GL_TEXTURE_3D, this->volume_text);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, this->width, this->height, this->depth, 0, GL_RED, GL_UNSIGNED_SHORT, texture_data);
			delete[] texture_data;
		}
		else
		{
			std::cout << "Error: dimensiones erroneas" << std::endl;
			return;
		}
}

void volume::create_quad_light_volume()
{
	GLfloat quad_data[] = {
		-0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.0f, 1.0f,
	};

	glBindVertexArray(this->texture_vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->texture_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), &quad_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

volume::~volume()
{
	glDeleteTextures(1, &this->volume_text);
}

volume_render::volume_render(int g_width, int g_height)
{
	this->g_width = g_width;
	this->g_height = g_height;
	this->init_shaders();
	this->unitary_cube = new cube();
	this->index_select = -1;
	this->press_volume_right = false;
	this->press_volume_left = false;
	glGenTextures(1, &this->transfer_function_text);
	glGenTextures(1, &this->backface_text);
	glGenFramebuffers(1, &this->frame_buffer);
	this->create_backface_text();
	this->create_frame_buffer();
	this->visible_interface = false;
	this->volume_interface = interface_volume::instance();
}

volume_render::~volume_render()
{
	glDeleteTextures(1, &this->backface_text);
	glDeleteTextures(1, &this->transfer_function_text);
	glDeleteFramebuffers(1, &this->frame_buffer);
	this->unitary_cube->~cube();
	for (unsigned int i = 0; i < this->volumes.size(); ++i) this->volumes[i]->~volume();
}

void volume_render::drop_path(int count, const char** paths)
{
	glm::uvec4 parameters;
	for (int i = 0; i < count; i++)
	{
		std::string path(paths[i]);
		if (this->process_path(path))
		{
			parameters = this->get_parameters(path);
			if (parameters != glm::uvec4(0))
			{
				volume *new_volume = new volume(path, parameters[0], parameters[1], parameters[2], parameters[3]);
				this->volumes.push_back(new_volume);
				this->index_select = this->volumes.size() - 1;
			}
			else
				std::cout << "Error cargando volumen" << std::endl;
		}
	}
}

bool volume_render::process_path(std::string path)
{
	std::string new_path = path.substr(path.find_last_of('\\')).erase(0, 1);
	new_path = new_path.substr(0, new_path.find(".raw"));
	for (unsigned int i = 0; i < this->volumes.size(); i++)
		if (new_path.compare(this->volumes[i]->name) == 0)
			return false;
	return true;
}

glm::uvec4 volume_render::get_parameters(std::string path)
{
	std::string new_path;
	std::vector<std::string> parameters, dimensions;
	new_path = path.substr(path.find_last_of('\\')).erase(0, 1);
	new_path = new_path.substr(0, new_path.find(".raw"));
	parameters = split(new_path, '_');
	if (parameters.size() == 3)
	{
		dimensions = split(parameters[1], 'x');
		if (dimensions.size() == 3)
			return glm::uvec4(std::stoi(dimensions[0]), std::stoi(dimensions[1]), std::stoi(dimensions[2]), std::stoi(parameters[2]));
	}
	return glm::uvec4(0);
}

void volume_render::init_shaders()
{
	this->backface.loadShader("Shaders/backFace.vert", CGLSLProgram::VERTEX);
	this->backface.loadShader("Shaders/backFace.frag", CGLSLProgram::FRAGMENT);
	this->raycasting.loadShader("Shaders/rayCasting.vert", CGLSLProgram::VERTEX);
	this->raycasting.loadShader("Shaders/rayCasting.frag", CGLSLProgram::FRAGMENT);
	this->lightcube.loadShader("Shaders/lightCube.vert", CGLSLProgram::VERTEX);
	this->lightcube.loadShader("Shaders/lightCube.frag", CGLSLProgram::FRAGMENT);

	this->backface.create_link();
	this->raycasting.create_link();
	this->lightcube.create_link();

	this->backface.enable();
	this->backface.addAttribute("vertex_coords");
	this->backface.addAttribute("volume_coords");
	this->backface.addUniform("MVP");
	this->backface.disable();

	this->raycasting.enable();
	this->raycasting.addAttribute("vertex_coords");
	this->raycasting.addAttribute("volume_coords");
	this->raycasting.addUniform("MVP");
	this->raycasting.addUniform("model");
	this->raycasting.addUniform("screen_size");
	this->raycasting.addUniform("step_size");
	this->raycasting.addUniform("light_pos");
	this->raycasting.addUniform("lighting");
	this->raycasting.addUniform("camera_pos");
	this->raycasting.addUniform("radius");
	this->raycasting.addUniform("asymmetry_param_g");
	this->raycasting.addUniform("back_radiance");
	this->raycasting.addUniform("ambient_comp");
	this->raycasting.addUniform("diffuse_comp");
	this->raycasting.addUniform("specular_comp");
	this->raycasting.addUniform("back_face_text");
	this->raycasting.addUniform("volume_text");
	this->raycasting.addUniform("transfer_function_text");
	this->raycasting.addUniform("scattering_coeff");
	this->raycasting.addUniform("extinction_coeff");
	this->raycasting.disable();

	this->lightcube.enable();
	this->lightcube.addAttribute("vertex_coords");
	this->lightcube.addAttribute("volume_coords");
	this->lightcube.addUniform("MVP");
	this->lightcube.addUniform("model_matrix");
	this->lightcube.addUniform("vp_matrix");
	this->lightcube.addUniform("light_pos");
	this->lightcube.addUniform("axis");
	this->lightcube.addUniform("start_texture");
	this->lightcube.addUniform("position");
	this->lightcube.addUniform("iteration");
	this->lightcube.addUniform("actual_texture");
	this->lightcube.addUniform("volume_text");
	this->lightcube.addUniform("previous_text");
	this->lightcube.addUniform("transfer_function_text");
	this->lightcube.disable();
}

void volume_render::scroll_volume(double y_offset)
{
	if (this->index_select != -1)
	{
		if (y_offset == 1)
			if (this->volumes[this->index_select]->escalation <= 3.0) this->volumes[this->index_select]->escalation += 0.05f;
		if (y_offset == -1)
			if (this->volumes[this->index_select]->escalation >= 0.05)	this->volumes[this->index_select]->escalation -= 0.05f;
	}
}

bool volume_render::click_volume(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position, bool type)
{
	if (this->index_select != -1)
	{
		if (this->intersection(x, y, projection, view, camera_position))
		{
			if (type)
				this->press_volume_right = true;
			else
				this->press_volume_left = true;
			this->x_reference = x;
			this->y_reference = y;

			this->volume_interface->show();
			this->volume_interface->translation = this->volumes[this->index_select]->translation;
			this->volume_interface->rotation = this->volumes[this->index_select]->rotation;
			this->volume_interface->scale = this->volumes[this->index_select]->escalation;
			this->volume_interface->asymmetry_param_g = this->volumes[this->index_select]->asymmetry_param_g;
			this->volume_interface->radius = this->volumes[this->index_select]->radius;
			this->volume_interface->back_radiance = this->volumes[this->index_select]->back_radiance;
			this->volume_interface->scattering_coeff = this->volumes[this->index_select]->scattering_coeff;
			this->volume_interface->extinction_coeff = this->volumes[this->index_select]->extinction_coeff;
			this->visible_interface = true;

			return true;
		}
	}
	return false;
}

bool volume_render::pos_cursor_volume(double x, double y)
{
	if (this->index_select != -1)
	{
		GLfloat offset_x, offset_y, angle, square;
		glm::quat quaternion;
		glm::vec3 axis;
		if (press_volume_right)
		{
			offset_x = (GLfloat)(x - this->x_reference);
			offset_y = (GLfloat)(y - this->y_reference);
			square = (offset_x * offset_x) + (offset_y * offset_y);
			angle = sqrt(square) * 0.15f;
			axis = glm::vec3(offset_y, offset_x, 0.0f);
			if (glm::length(axis) != 0.0f)
			{
				axis = glm::normalize(axis);
				quaternion = glm::angleAxis(angle, axis);
				this->volumes[this->index_select]->rotation = glm::cross(quaternion, this->volumes[this->index_select]->rotation);
			}
			this->x_reference = x;
			this->y_reference = y;
			return true;
		}
		if (press_volume_left)
		{
			if (x > this->x_reference) this->volumes[this->index_select]->translation.x += (GLfloat)(x - this->x_reference) * 0.002f;
			else if (x < this->x_reference) this->volumes[this->index_select]->translation.x -= (GLfloat)(this->x_reference - x) * 0.002f;
			if (y > this->y_reference) this->volumes[this->index_select]->translation.y -= (GLfloat)(y - this->y_reference) * 0.002f;
			else if (y < this->y_reference) this->volumes[this->index_select]->translation.y += (GLfloat)(this->y_reference - y) * 0.002f;
			this->x_reference = x;
			this->y_reference = y;
			return true;
		}
	}
	return false;
}

void volume_render::disable_select()
{
	this->press_volume_right = false;
	this->press_volume_left = false;
}

glm::vec4 volume_render::calculate_dir_max(glm::vec3 light_pos, glm::mat4 model)
{
	float f_0, aux;
	unsigned int index, axis;
	glm::mat3 normal_matrix;
	std::vector<glm::vec3> center_cube_faces, normals_cube_faces, dir_max;
	
	normal_matrix = glm::transpose(glm::inverse(glm::mat3(model)));
	f_0 = 0.0f;

	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(0.0f, 0.5f, 0.0f), 1.0f)));
	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(0.5f, 0.0f, 0.0f), 1.0f)));
	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(0.0f, 0.0f, -0.5f), 1.0f)));
	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(-0.5f, 0.0f, 0.0f), 1.0f)));
	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(0.0f, 0.0f, 0.5f), 1.0f)));
	center_cube_faces.push_back(glm::vec3(model * glm::vec4(glm::vec3(0.0f, -0.5f, 0.0f), 1.0f)));

	normals_cube_faces.push_back(normal_matrix * glm::vec3(0.0f, 1.0f, 0.0f));  // Revisar da al reves
	normals_cube_faces.push_back(normal_matrix * glm::vec3(1.0f, 0.0f, 0.0f));
	normals_cube_faces.push_back(normal_matrix * glm::vec3(0.0f, 0.0f, -1.0f));
	normals_cube_faces.push_back(normal_matrix * glm::vec3(-1.0f, 0.0f, 0.0f));
	normals_cube_faces.push_back(normal_matrix * glm::vec3(0.0f, 0.0f, 1.0f));
	normals_cube_faces.push_back(normal_matrix * glm::vec3(0.0f, -1.0f, 0.0f));

	for (size_t i = 0; i < 6; i++)
	{
		aux = glm::dot(glm::normalize(normals_cube_faces[i]), glm::normalize((light_pos - center_cube_faces[i])));
		if (aux < 0)
		{
			if (f_0 > aux)
			{
				f_0 = aux;
				index = i;
			}
		}
	}
	
	if (index == 0 || index == 5)
	{
		axis = 1;
		this->volumes[this->index_select]->step_light_volume = 1.0f / this->volumes[this->index_select]->height;
	}
	else if (index == 1 || index == 3)
	{
		axis = 0;
		this->volumes[this->index_select]->step_light_volume = 1.0f / this->volumes[this->index_select]->width;
	}
	else if (index == 2 || index == 4)
	{
		axis = 2;
		this->volumes[this->index_select]->step_light_volume = 1.0f / this->volumes[this->index_select]->depth;
	}
	this->volumes[this->index_select]->current_index = index;
	return glm::vec4(glm::normalize(-normals_cube_faces[index]), axis);
}

glm::vec4 volume_render::get_position(int index)
{
	int sign;
	switch (index)
	{
		case 0:
			return glm::vec4(0.0f, 0.5f, 0.0f, -1.0f);
			break;
		case 1:
			return glm::vec4(0.5f, 0.0f, 0.0f, -1.0f);
			break;
		case 2:
			return glm::vec4(0.0f, 0.0f, -0.5f, 1.0f);
			break;
		case 3:
			return glm::vec4(-0.5f, 0.0f, 0.0f, 1.0f);
			break;
		case 4:
			return glm::vec4(0.0f, 0.0f, 0.5f, -1.0f);
			break;
		case 5:
			return glm::vec4(0.0f, -0.5f, 0.0f, 1.0f);
			break;
	}
}

bool volume_render::intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 camera_position)
{
	GLfloat ax, ay, tx1, tx2, tmin, tmax, amin, amax, ty1, ty2, tz1, tz2;
	glm::vec4 camera_position2, ray_clip, ray_eye, min, max;
	glm::vec3 ray_world;
	glm::mat4 model;

	ax = (GLfloat)((2.0f * x) / this->g_width - 1.0f);
	ay = (GLfloat)(1.0f - (2.0f * y) / this->g_height);

	ray_clip = glm::vec4(ax, ay, -1.0f, 1.0f);
	ray_eye = glm::inverse(projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	ray_eye = glm::inverse(view) * ray_eye;

	model = glm::translate(glm::mat4(1.0f), this->volumes[this->index_select]->translation) * glm::mat4_cast(this->volumes[this->index_select]->rotation) * glm::scale(model, glm::vec3(this->volumes[this->index_select]->escalation));
	ray_world = glm::vec3(glm::normalize(glm::inverse(model) * glm::vec4(ray_eye.x, ray_eye.y, ray_eye.z, 0.0f)));
	camera_position2 = glm::inverse(model) * glm::vec4(camera_position, 1.0f);

	min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
	max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	ray_world = glm::vec3(1.0 / ray_world.x, 1.0 / ray_world.y, 1.0 / ray_world.z);

	tx1 = (min.x - camera_position2.x) * ray_world.x;
	tx2 = (max.x - camera_position2.x) * ray_world.x;
	tmin = glm::min(tx1, tx2);
	tmax = glm::max(tx1, tx2);
	amin = tmin, amax = tmax;

	ty1 = (min.y - camera_position2.y) * ray_world.y;
	ty2 = (max.y - camera_position2.y) * ray_world.y;
	amin = glm::min(ty1, ty2);
	tmin = glm::max(amin, tmin);
	amax = glm::max(ty1, ty2);
	tmax = glm::min(tmax, amax);

	tz1 = (min.z - camera_position2.z) * ray_world.z;
	tz2 = (max.z - camera_position2.z) * ray_world.z;
	amin = glm::min(tz1, tz2);
	tmin = glm::max(amin, tmin);
	amax = glm::max(tz1, tz2);
	tmax = glm::min(tmax, amax);

	if (tmax >= tmin && tmin < FLT_MAX) return true;
	return false;
}

void volume_render::load_transfer_func_t(GLfloat data[][4])
{
	glBindTexture(GL_TEXTURE_1D, this->transfer_function_text);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, data);
}

void volume_render::update_transfer_function(std::vector<double> points)
{
	GLfloat trans_funct_aux[256][4];
	GLfloat difference_r, difference_g, difference_b, difference_a;
	std::vector<unsigned int> index;
	unsigned int i, actual, j, difference_index;

	points[0] = 0.0;
	points[points.size() - 5] = 1.0;

	for (i = 0; i < points.size(); i += 5)
	{
		actual = (unsigned int)(points[i] * 255u);
		trans_funct_aux[actual][0] = GLfloat(points[i + 1]);
		trans_funct_aux[actual][1] = GLfloat(points[i + 2]);
		trans_funct_aux[actual][2] = GLfloat(points[i + 3]);
		trans_funct_aux[actual][3] = GLfloat(points[i + 4]);
		index.push_back(actual);
	}

	for (j = 0; j < (points.size() / 5) - 1; j++)
	{
		difference_r = trans_funct_aux[index[j + 1]][0] - trans_funct_aux[index[j]][0];
		difference_g = trans_funct_aux[index[j + 1]][1] - trans_funct_aux[index[j]][1];
		difference_b = trans_funct_aux[index[j + 1]][2] - trans_funct_aux[index[j]][2];
		difference_a = trans_funct_aux[index[j + 1]][3] - trans_funct_aux[index[j]][3];
		difference_index = index[j + 1] - index[j];

		difference_r /= GLfloat(difference_index);
		difference_g /= GLfloat(difference_index);
		difference_b /= GLfloat(difference_index);
		difference_a /= GLfloat(difference_index);

		for (i = index[j] + 1; i < index[j + 1]; i++)
		{
			trans_funct_aux[i][0] = (trans_funct_aux[i - 1][0] + difference_r);
			trans_funct_aux[i][1] = (trans_funct_aux[i - 1][1] + difference_g);
			trans_funct_aux[i][2] = (trans_funct_aux[i - 1][2] + difference_b);
			trans_funct_aux[i][3] = (trans_funct_aux[i - 1][3] + difference_a);
		}
	}
	this->load_transfer_func_t(trans_funct_aux);
}

void volume_render::create_backface_text()
{
	glBindTexture(GL_TEXTURE_2D, this->backface_text);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->g_width, this->g_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool volume_render::create_frame_buffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->backface_text, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void volume_render::render_cube(glm::mat4 &MVP)
{
	this->backface.enable();
	glUniformMatrix4fv(this->backface.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	this->unitary_cube->display();
	this->backface.disable();
}

void volume_render::render_cube_raycast(glm::mat4 &MVP, glm::mat4 &model, glm::vec3 view_pos, light* scene_lights, glm::mat4 view_projection)
{
	if (true) //Recordar poner condiciones de transfer_function y light_pos 
	{
		int actual_texture;
		glm::vec4 position_sign, dir_max;
		glm::vec3 ray_step, position;
		float distance, lenght_in_out, step_size, texture_step, start_texture;

		actual_texture = 1;
		dir_max = this->calculate_dir_max(scene_lights->translation, model);
		step_size = this->volumes[this->index_select]->step_light_volume;
		position_sign = this->get_position(this->volumes[this->index_select]->current_index);
		ray_step = (glm::vec3(dir_max.x, dir_max.y, dir_max.z) * step_size) * position_sign.w;
		texture_step = (step_size) * position_sign.w;
		position = glm::vec3(position_sign.x, position_sign.y, position_sign.z);

		if (position_sign.w == -1.0f)
			start_texture = 1.0f;
		else
			start_texture = 0.0f;
		
		this->lightcube.enable();
		glBindFramebuffer(GL_FRAMEBUFFER, this->volumes[this->index_select]->volume_buffer);
		lenght_in_out = glm::length(glm::vec3(dir_max.x, dir_max.y, dir_max.z));
		for (float i = 0.0f; i < lenght_in_out; i+=step_size)
		{
			if (actual_texture == 1)
				actual_texture = 0;
			else
				actual_texture = 1;

			glUniformMatrix4fv(this->lightcube.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, this->transfer_function_text);
			glUniform1i(this->lightcube.getLocation("transfer_function_text"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, this->volumes[this->index_select]->volume_text);
			glUniform1i(this->lightcube.getLocation("volume_text"), 1);
			if (i > 0.0f)
			{
				glActiveTexture(GL_TEXTURE2);
				if (actual_texture == 0)
					glBindTexture(GL_TEXTURE_2D, this->volumes[this->index_select]->previous_texture);
				else
					glBindTexture(GL_TEXTURE_2D, this->volumes[this->index_select]->render_texture);
				glUniform1i(this->lightcube.getLocation("previous_text"), 2);
			}
			glUniformMatrix4fv(this->lightcube.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(this->lightcube.getLocation("vp_matrix"), 1, GL_FALSE, glm::value_ptr(view_projection));
			glUniform1i(this->lightcube.getLocation("actual_texture"), actual_texture);
			glUniform1i(this->lightcube.getLocation("axis"), dir_max.w);
			glUniform1f(this->lightcube.getLocation("start_texture"), start_texture);
			glUniform3fv(this->lightcube.getLocation("light_pos"), 1, glm::value_ptr(scene_lights->translation));
			glUniform3fv(this->lightcube.getLocation("normal"), 1, glm::value_ptr(-glm::vec3(dir_max.x, dir_max.y, dir_max.z)));
			glUniform3fv(this->lightcube.getLocation("position"), 1, &position[0]);
			glUniform1f(this->lightcube.getLocation("iteration"), i);

			glBindVertexArray(this->volumes[this->index_select]->texture_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);

			position += ray_step;
			start_texture += texture_step;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		this->lightcube.disable();
	}

	this->raycasting.enable();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, this->transfer_function_text);
	glUniform1i(this->raycasting.getLocation("transfer_function_text"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->backface_text);
	glUniform1i(this->raycasting.getLocation("back_face_text"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, this->volumes[this->index_select]->volume_text);
	glUniform1i(this->raycasting.getLocation("volume_text"), 2);
	glUniform2f(this->raycasting.getLocation("screen_size"), (GLfloat)this->g_width, (GLfloat)this->g_height);
	glUniform1f(this->raycasting.getLocation("step_size"), this->volumes[this->index_select]->step);
	glUniformMatrix4fv(this->raycasting.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(this->raycasting.getLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(this->raycasting.getLocation("camera_pos"), 1, &view_pos[0]);
	glUniform1f(this->raycasting.getLocation("radius"), this->volumes[this->index_select]->radius);
	glUniform1f(this->raycasting.getLocation("asymmetry_param_g"), this->volumes[this->index_select]->asymmetry_param_g);
	glUniform4fv(this->raycasting.getLocation("back_radiance"), 1, &this->volumes[this->index_select]->back_radiance[0]);
	glUniform3fv(this->raycasting.getLocation("light_pos"), 1, glm::value_ptr(scene_lights->translation));
	glUniform3fv(this->raycasting.getLocation("ambient_comp"), 1, glm::value_ptr(scene_lights->ambient_comp));
	glUniform3fv(this->raycasting.getLocation("diffuse_comp"), 1, glm::value_ptr(scene_lights->diffuse_comp));
	glUniform3fv(this->raycasting.getLocation("specular_comp"), 1, glm::value_ptr(scene_lights->specular_comp));
	glUniform3fv(this->raycasting.getLocation("scattering_coeff"), 1, &this->volumes[this->index_select]->scattering_coeff[0]);
	glUniform3fv(this->raycasting.getLocation("extinction_coeff"), 1, &this->volumes[this->index_select]->extinction_coeff[0]);
	glUniform1i(this->raycasting.getLocation("lighting"), (GLint)(scene_lights->on));
	this->unitary_cube->display();
	this->raycasting.disable();
}

void volume_render::display(glm::mat4 &view_projection, glm::vec3 view_pos, light* scene_lights)
{
	if (this->index_select != -1)
	{
		glm::mat4 model, MVP;
		model = glm::translate(glm::mat4(1.0f), this->volumes[this->index_select]->translation) * glm::mat4_cast(this->volumes[this->index_select]->rotation) * glm::scale(model, glm::vec3(this->volumes[this->index_select]->escalation));
		MVP = view_projection * model;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, this->frame_buffer);
		glViewport(0, 0, this->g_width, this->g_height);
		this->render_cube(MVP);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, this->g_width, this->g_height);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		this->render_cube_raycast(MVP, model, view_pos, scene_lights, view_projection);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
}

void volume_render::change_volume(int type)
{
	if (this->index_select != -1)
	{
		if (type == 0)
			if (this->index_select > 0)
				this->index_select--;
			else
				this->index_select = this->volumes.size() - 1;
		else
			if (type == 1)
				if (this->index_select < this->volumes.size() - 1)
					this->index_select++;
				else
					this->index_select = 0;
	}
}

void volume_render::resize_screen(const glm::vec2 screen)
{
	this->g_width = (int)screen.x;
	this->g_height = (int)screen.y;
	this->create_backface_text();
	this->create_frame_buffer();
}

void volume_render::update_interface()
{
	if (visible_interface)
	{
		if (this->volumes[this->index_select]->translation != this->volume_interface->translation)
		{
			this->volumes[this->index_select]->translation = this->volume_interface->translation;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->rotation != this->volume_interface->rotation)
		{
			this->volumes[this->index_select]->rotation = this->volume_interface->rotation;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->escalation != this->volume_interface->scale)
		{
			this->volumes[this->index_select]->escalation = this->volume_interface->scale;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->asymmetry_param_g != this->volume_interface->asymmetry_param_g)
		{
			this->volumes[this->index_select]->asymmetry_param_g = this->volume_interface->asymmetry_param_g;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->radius != this->volume_interface->radius)
		{
			this->volumes[this->index_select]->radius = this->volume_interface->radius;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->back_radiance != this->volume_interface->back_radiance)
		{
			this->volumes[this->index_select]->back_radiance = this->volume_interface->back_radiance;
			this->volumes[this->index_select]->change_values = true;
		}	
		if (this->volumes[this->index_select]->scattering_coeff != this->volume_interface->scattering_coeff)
		{
			this->volumes[this->index_select]->scattering_coeff = this->volume_interface->scattering_coeff;
			this->volumes[this->index_select]->change_values = true;
		}
		if (this->volumes[this->index_select]->extinction_coeff != this->volume_interface->extinction_coeff)
		{
			this->volumes[this->index_select]->extinction_coeff = this->volume_interface->extinction_coeff;
			this->volumes[this->index_select]->change_values = true;
		}
	}
}