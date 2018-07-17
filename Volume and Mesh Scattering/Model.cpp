#include "Model.h"

mesh::mesh()
{
	this->translation = glm::vec3(-3.0f, 0.0f, 0.0f);
	this->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->scale = 4.0f;
	this->ambient_comp = glm::vec3(0.15f, 0.15f, 0.15f);
	this->diffuse_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->specular_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->asymmetry_param_g = -1.0f;
	this->refractive_index = 1.3f;
	this->current_material = Crema;
	this->q = 1.0f;
	this->radius = 0.025f;
	this->epsilon = 0.04f;
	this->gamma = 1.25f;
	this->bias = 0.005f;
	this->change_values = true;
	this->visible_interface = false;
	this->min_vertex = glm::vec3(1000);
	this->max_vertex = glm::vec3(-1000);
	this->model_interface = interface_model::instance();
}

mesh::~mesh()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void mesh::set_max_min_value(GLfloat x, GLfloat y, GLfloat z)
{
	if (x > this->max_vertex.x) this->max_vertex.x = x; if (y > this->max_vertex.y) this->max_vertex.y = y; if (z > this->max_vertex.z) this->max_vertex.z = z;
	if (x < this->min_vertex.x) this->min_vertex.x = x; if (y < this->min_vertex.y) this->min_vertex.y = y; if (z < this->min_vertex.z) this->min_vertex.z = z;
}

std::vector<std::string> mesh::process_attribute(std::string value)
{
	std::string output;
	unique_copy(value.begin(), value.end(), std::back_insert_iterator<std::string>(output), [](char a, char b) { return isspace(a) && isspace(b); });
	return split(output.erase(0, 1), ' ');
}

std::vector<std::string> mesh::split(std::string s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (getline(ss, item, delim))
		tokens.push_back(item);
	return tokens;
}

void mesh::calculate_center()
{
	this->center.x = (this->max_vertex.x + this->min_vertex.x) / 2.0f;
	this->center.y = (this->max_vertex.y + this->min_vertex.y) / 2.0f;
	this->center.z = (this->max_vertex.z + this->min_vertex.z) / 2.0f;
	this->max_value = max(this->max_vertex.x, max(this->max_vertex.y, this->max_vertex.z));
}

void mesh::calculate_normals(std::vector<glm::vec3> &aux_normals, std::vector<glm::vec3> aux_vertices, std::vector<glm::uvec3> index_vertices) 
{
	std::vector<glm::vec3> normals(aux_vertices.size(), glm::vec3(0.0f));
	std::vector<float> totals(aux_vertices.size(), 0.0f);
	glm::vec3 u, v, v1, v2, v3, normal;

	for (size_t i = 0; i < index_vertices.size(); i++) 
	{
		v1 = aux_vertices[index_vertices[i].x];
		v2 = aux_vertices[index_vertices[i].y];
		v3 = aux_vertices[index_vertices[i].z];

		u = v1 - v3;
		v = v1 - v2;
		normal = glm::cross(u, v);
		normal = glm::normalize(normal);
		normals[index_vertices[i].x] += normal;
		normals[index_vertices[i].y] += normal;
		normals[index_vertices[i].z] += normal;

		totals[index_vertices[i].x] += 1.0f;
		totals[index_vertices[i].y] += 1.0f;
		totals[index_vertices[i].z] += 1.0f;
	}

	for (size_t i = 0; i < normals.size(); i++)
		aux_normals.push_back(-normals[i] / glm::vec3(totals[i]));
}

void mesh::load(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in);
	if (file.is_open())
	{
		std::string buffer;
		std::vector<glm::vec3> aux_vertices, aux_normals;
		std::vector<unsigned int> aux_index_vertices, aux_index_normals;
		std::vector<glm::uvec3> index_vertices, index_normals;
		glm::uvec3 vertex_aux_index, normal_aux_index;
		bool vertex_normals;
		GLfloat x, y, z;

		while (!file.eof())
		{
			file >> buffer;
			if (buffer == "v")
			{
				file >> x >> y >> z;
				aux_vertices.push_back(glm::vec3(x, y, z));
				set_max_min_value(x, y, z);
			}
			else if (buffer == "vn")
			{
				file >> x >> y >> z;
				aux_normals.push_back(glm::vec3(x, y, z));
			}
			else if (buffer == "f")
			{
				getline(file, buffer);
				std::vector<std::string> tokens, aux_tokens;

				if (buffer.find('/', 0) == std::string::npos)
				{
					tokens = process_attribute(buffer);
					for (unsigned int i = 1; (i + 1) < tokens.size(); i++)
						index_vertices.push_back(glm::uvec3((atoi(tokens[0].c_str()) - 1), (atoi(tokens[i].c_str()) - 1), (atoi(tokens[i + 1].c_str()) - 1)));
				}
				else
				{
					vertex_normals = false;
					if (buffer.find("//", 0) != std::string::npos)
						vertex_normals = true;

					tokens = process_attribute(buffer);
					aux_index_vertices.clear();
					aux_index_normals.clear();
					for (unsigned int i = 0; i < tokens.size(); i++)
					{
						if (!vertex_normals)
							aux_tokens = split(tokens[i], '/');
						else
						{
							tokens[i].erase(tokens[i].find('/'), 1);
							aux_tokens = split(tokens[i], '/');
							aux_index_normals.push_back((atoi(aux_tokens[1].c_str()) - 1));
						}
						aux_index_vertices.push_back((atoi(aux_tokens[0].c_str()) - 1));
						if (aux_tokens.size() == 3)
							aux_index_normals.push_back((atoi(aux_tokens[2].c_str()) - 1));
					}

					for (size_t i = 1; (i + 1) < aux_index_vertices.size(); i++)
					{
						index_vertices.push_back(glm::uvec3(aux_index_vertices[0], aux_index_vertices[i], aux_index_vertices[i + 1]));
						index_normals.push_back(glm::uvec3(aux_index_normals[0], aux_index_normals[i], aux_index_normals[i + 1]));
					}
				}
			}
		}

		file.close();

		vertices = std::vector<glm::vec3>(index_vertices.size() * 3);
		normals = std::vector<glm::vec3>(index_vertices.size() * 3);
		calculate_center();

		if (aux_normals.empty()) {
			this->calculate_normals(aux_normals, aux_vertices, index_vertices);

			for (size_t i = 0; i < index_vertices.size(); i++)
			{
				vertex_aux_index = index_vertices[i];
				this->vertices[(i * 3)] = (aux_vertices[vertex_aux_index.x] - this->center) / this->max_value;
				this->vertices[(i * 3) + 1] = (aux_vertices[vertex_aux_index.y] - this->center) / this->max_value;
				this->vertices[(i * 3) + 2] = (aux_vertices[vertex_aux_index.z] - this->center) / this->max_value;

				this->normals[(i * 3)] = aux_normals[vertex_aux_index.x];
				this->normals[(i * 3) + 1] = aux_normals[vertex_aux_index.y];
				this->normals[(i * 3) + 2] = aux_normals[vertex_aux_index.z];
			}
		}
		else {
			for (size_t i = 0; i < index_vertices.size(); i++)
			{
				vertex_aux_index = index_vertices[i];
				this->vertices[(i * 3)] = (aux_vertices[vertex_aux_index.x] - this->center) / this->max_value;
				this->vertices[(i * 3) + 1] = (aux_vertices[vertex_aux_index.y] - this->center) / this->max_value;
				this->vertices[(i * 3) + 2] = (aux_vertices[vertex_aux_index.z] - this->center) / this->max_value;

				normal_aux_index = index_normals[i];
				this->normals[(i * 3)] = aux_normals[normal_aux_index.x];
				this->normals[(i * 3) + 1] = aux_normals[normal_aux_index.y];
				this->normals[(i * 3) + 2] = aux_normals[normal_aux_index.z];
			}
		}

		aux_index_vertices.clear();
		aux_index_normals.clear();
		index_vertices.clear();
		index_normals.clear();
		aux_vertices.clear();
		aux_normals.clear();

		this->max_vertex = (this->max_vertex - this->center) / this->max_value;
		this->min_vertex = (this->min_vertex - this->center) / this->max_value;
		calculate_center();

		this->bounding_box.push_back(this->max_vertex);
		this->bounding_box.push_back(this->min_vertex);
		this->bounding_box.push_back(glm::vec3(this->min_vertex.x, this->min_vertex.y, this->max_vertex.z));
		this->bounding_box.push_back(glm::vec3(this->min_vertex.x, this->max_vertex.y, this->max_vertex.z));
		this->bounding_box.push_back(glm::vec3(this->max_vertex.x, this->max_vertex.y, this->min_vertex.z));
		this->bounding_box.push_back(glm::vec3(this->min_vertex.x, this->max_vertex.y, this->min_vertex.z));
		this->bounding_box.push_back(glm::vec3(this->max_vertex.x, this->min_vertex.y, this->min_vertex.z));
		this->bounding_box.push_back(glm::vec3(this->max_vertex.x, this->min_vertex.y, this->max_vertex.z));

		this->create_vbo();
	}
}

void mesh::create_vbo()
{
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

	glBufferData(GL_ARRAY_BUFFER, (this->vertices.size() + this->normals.size()) * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->vertices.size() * sizeof(glm::vec3), &this->vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), this->normals.size() * sizeof(glm::vec3), &this->normals[0]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(this->vertices.size() * sizeof(glm::vec3)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void mesh::click_model()
{
	this->model_interface->show();
	this->model_interface->translation = this->translation;
	this->model_interface->rotation = this->rotation;
	this->model_interface->scale = this->scale;
	this->model_interface->asymmetry_param_g = this->asymmetry_param_g;
	this->model_interface->current_material = this->current_material;
	this->model_interface->q = this->q;
	this->model_interface->radius = this->radius;
	this->model_interface->gamma = this->gamma;
	this->model_interface->bias = this->bias;
	this->model_interface->epsilon = this->epsilon;
	this->visible_interface = true;
}

void mesh::not_click_model()
{
	this->model_interface->hide();
	this->visible_interface = false;
}

void mesh::update_interface()
{
	if (visible_interface)
	{
		glm::mat4 model_mat;
		if (this->translation != this->model_interface->translation)
		{
			model_mat = glm::mat4(1.0f);
			model_mat = glm::translate(model_mat, this->model_interface->translation);
			model_mat = model_mat * glm::toMat4(this->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(this->scale));
			if (this->no_collision(model_mat))
			{
				this->translation = this->model_interface->translation;
				this->change_values = true;
			}
			else
				this->model_interface->translation = this->translation;
		}
		if (this->rotation != this->model_interface->rotation)
		{
			model_mat = glm::mat4(1.0f);
			model_mat = glm::translate(model_mat, this->translation);
			model_mat = model_mat * glm::toMat4(this->model_interface->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(this->scale));
			if (this->no_collision(model_mat))
			{
				this->rotation = this->model_interface->rotation;
				this->change_values = true;
			}
			else
				this->model_interface->rotation = this->rotation;
		}
		if (this->scale != this->model_interface->scale)
		{
			model_mat = glm::mat4(1.0f);
			model_mat = glm::translate(model_mat, this->translation);
			model_mat = model_mat * glm::toMat4(this->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(this->model_interface->scale));
			if (this->no_collision(model_mat))
			{
				this->scale = this->model_interface->scale;
				this->change_values = true;
			}
			else
				this->model_interface->scale = this->scale;
		}
		if (this->current_material != this->model_interface->current_material) {
			switch (this->model_interface->current_material) {
			case Patata:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.005f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			case Mármol:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.025f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			case Leche:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.025f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			case Crema:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.025f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			case Piel:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.005f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			case Ninguno:
				this->model_interface->asymmetry_param_g = -1.0f;
				this->model_interface->radius = 0.025f;
				this->model_interface->gamma = 1.25f;
				this->model_interface->epsilon = 0.04f;
				this->model_interface->bias = 0.005f;
				break;
			}
		}
		if (this->asymmetry_param_g != this->model_interface->asymmetry_param_g)
		{
			this->asymmetry_param_g = this->model_interface->asymmetry_param_g;
			this->change_values = true;
		}
		if (this->radius != this->model_interface->radius)
		{
			this->radius = this->model_interface->radius;
			this->change_values = true;
		}
		if (this->epsilon != this->model_interface->epsilon)
		{
			this->epsilon = this->model_interface->epsilon;
			this->change_values = true;
		}
		if (this->gamma != this->model_interface->gamma)
		{
			this->gamma = this->model_interface->gamma;
			this->change_values = true;
		}
		if (this->bias != this->model_interface->bias)
		{
			this->bias = this->model_interface->bias;
			this->change_values = true;
		}
		if (this->current_material != this->model_interface->current_material)
		{
			this->current_material = this->model_interface->current_material;
			this->change_values = true;
		}
	}
}

bool mesh::no_collision(glm::mat4 &model)
{
	GLfloat bound = 15.0f;
	glm::vec3 aux, new_max, new_min;

	new_min = glm::vec3(1000);
	new_max = glm::vec3(-1000);

	for (size_t i = 0; i < this->bounding_box.size(); i++)
	{
		aux = glm::vec3(model * glm::vec4(this->bounding_box[i], 1.0f));
		if (aux.x > new_max.x) new_max.x = aux.x; if (aux.y > new_max.y) new_max.y = aux.y; if (aux.z > new_max.z) new_max.z = aux.z;
		if (aux.x < new_min.x) new_min.x = aux.x; if (aux.y < new_min.y) new_min.y = aux.y; if (aux.z < new_min.z) new_min.z = aux.z;
	}

	return ((new_min.x >= -bound + 0.5f && new_max.x <= bound + 0.5f) &&
		(new_min.y >= -bound && new_max.y <= bound) &&
		(new_min.z >= -bound && new_max.z <= bound));
}

model::model(int g_width, int g_height) {
	material *potato, *marble, *skin, *milk, *cream, *none;

	this->scene_model = new mesh();
	this->halton_generator = new halton();
	this->materials = new materials_set();
	this->light_buffers = new light_buffer(g_width, g_height, 1);
	this->num_of_ortho_cameras = 32;
	this->num_of_samples_per_frag = 3 * this->num_of_ortho_cameras;
	this->selected_camera = 0;

	potato = new material(glm::vec3(0.68f, 0.70f, 0.55f), glm::vec3(0.0024f, 0.0090f, 0.12f), glm::vec3(0.77f, 0.62f, 0.21f), 1.3f);
	marble = new material(glm::vec3(2.19f, 2.62f, 3.00f), glm::vec3(0.0021f, 0.0041f, 0.0071f), glm::vec3(0.83f, 0.79f, 0.75f), 1.5f);
	skin = new material(glm::vec3(0.74f, 0.88f, 1.01f), glm::vec3(0.032f, 0.17f, 0.48f), glm::vec3(0.44f, 0.22f, 0.13f), 1.3f);
	milk = new material(glm::vec3(2.55f, 3.21f, 3.77f), glm::vec3(0.0011f, 0.0024f, 0.014f), glm::vec3(0.91f, 0.88f, 0.76f), 1.3f);
	cream = new material(glm::vec3(7.38f, 5.47f, 3.1f), glm::vec3(0.0002f, 0.0028f, 0.016f), glm::vec3(0.98f, 0.90f, 0.7f), 1.3f);
	none = new material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.3f);

	this->materials->materials.push_back(potato);
	this->materials->materials.push_back(marble);
	this->materials->materials.push_back(skin);
	this->materials->materials.push_back(milk);
	this->materials->materials.push_back(cream);
	this->materials->materials.push_back(none);

	this->scattered_maps = new scattered_map(g_width, g_height, this->num_of_ortho_cameras);
	this->scene_model->load("Models/obj/bunny.obj");
	this->halton_generator->generate_orthographic_cameras(this->num_of_ortho_cameras);
	this->init_shaders();
}

void model::init_shaders() {
	this->glsl_g_buffer.loadShader("Shaders/gBuffer.vert", CGLSLProgram::VERTEX);
	this->glsl_g_buffer.loadShader("Shaders/gBuffer.frag", CGLSLProgram::FRAGMENT);
	this->glsl_g_buffer.loadShader("Shaders/gBuffer.geom", CGLSLProgram::GEOMETRY);
	this->glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.vert", CGLSLProgram::VERTEX);
	this->glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.frag", CGLSLProgram::FRAGMENT);
	this->glsl_scattered_map.loadShader("Shaders/scatteredMap.vert", CGLSLProgram::VERTEX);
	this->glsl_scattered_map.loadShader("Shaders/scatteredMap.frag", CGLSLProgram::FRAGMENT);
	this->glsl_scattered_map.loadShader("Shaders/scatteredMap.geom", CGLSLProgram::GEOMETRY);
	this->glsl_blending.loadShader("Shaders/blending.vert", CGLSLProgram::VERTEX);
	this->glsl_blending.loadShader("Shaders/blending.frag", CGLSLProgram::FRAGMENT);
	this->glsl_phong.loadShader("Shaders/blinnPhong.vert", CGLSLProgram::VERTEX);
	this->glsl_phong.loadShader("Shaders/blinnPhong.frag", CGLSLProgram::FRAGMENT);

	this->glsl_g_buffer.create_link();
	this->glsl_g_buffer_plane.create_link();
	this->glsl_scattered_map.create_link();
	this->glsl_blending.create_link();
	this->glsl_phong.create_link();

	this->glsl_g_buffer.enable();
	this->glsl_g_buffer.addAttribute("position");
	this->glsl_g_buffer.addAttribute("normal");

	this->glsl_g_buffer.addUniform("model_matrix");
	this->glsl_g_buffer.addUniform("vp_light");
	this->glsl_g_buffer.addUniform("num_of_buffer");
	this->glsl_g_buffer.disable();

	this->glsl_g_buffer_plane.enable();
	this->glsl_g_buffer_plane.addAttribute("position");
	this->glsl_g_buffer_plane.addAttribute("tex_coords");

	this->glsl_g_buffer_plane.addUniform("model_matrix");
	this->glsl_g_buffer_plane.addUniform("texture");
	this->glsl_g_buffer_plane.addUniform("camera_select");
	this->glsl_g_buffer_plane.disable();

	this->glsl_scattered_map.enable();
	this->glsl_scattered_map.addAttribute("position");
	this->glsl_scattered_map.addAttribute("normal");

	this->glsl_scattered_map.addUniform("n_cameras");
	this->glsl_scattered_map.addUniform("cameras_matrix");
	this->glsl_scattered_map.addUniform("vp_light");
	this->glsl_scattered_map.addUniform("model_matrix");
	this->glsl_scattered_map.addUniform("model_center");
	this->glsl_scattered_map.addUniform("g_width");
	this->glsl_scattered_map.addUniform("g_height");

	this->glsl_scattered_map.addUniform("asymmetry_param_g");
	this->glsl_scattered_map.addUniform("light_pos");
	this->glsl_scattered_map.addUniform("n_samples");
	this->glsl_scattered_map.addUniform("samples");
	this->glsl_scattered_map.addUniform("g_position");
	this->glsl_scattered_map.addUniform("g_normal");
	this->glsl_scattered_map.addUniform("g_depth");
	this->glsl_scattered_map.addUniform("radius");
	this->glsl_scattered_map.addUniform("bias");
	this->glsl_scattered_map.addUniform("refractive_index");
	this->glsl_scattered_map.addUniform("diffuse_reflectance");
	this->glsl_scattered_map.addUniform("light_diffuse_color");

	// Valores pre-calculados
	this->glsl_scattered_map.addUniform("attenuation_coeff");
	this->glsl_scattered_map.addUniform("D");
	this->glsl_scattered_map.addUniform("effective_transport_coeff");
	this->glsl_scattered_map.addUniform("c_phi_1");
	this->glsl_scattered_map.addUniform("c_phi_2");
	this->glsl_scattered_map.addUniform("c_e");
	this->glsl_scattered_map.addUniform("A");
	this->glsl_scattered_map.addUniform("de");
	this->glsl_scattered_map.addUniform("zr");
	this->glsl_scattered_map.disable();

	this->glsl_blending.enable();
	this->glsl_blending.addAttribute("position");
	this->glsl_blending.addAttribute("normal");

	this->glsl_blending.addUniform("MVP");
	this->glsl_blending.addUniform("model_matrix");
	this->glsl_blending.addUniform("scattered_map");
	this->glsl_blending.addUniform("depth_map");
	this->glsl_blending.addUniform("camera_pos");
	this->glsl_blending.addUniform("light_pos");
	this->glsl_blending.addUniform("epsilon");
	this->glsl_blending.addUniform("refractive_index");
	this->glsl_blending.addUniform("n_cameras");
	this->glsl_blending.addUniform("cameras_matrix");
	this->glsl_blending.addUniform("cameras_dirs");
	this->glsl_blending.addUniform("gamma");
	this->glsl_blending.addUniform("bias");
	this->glsl_blending.addUniform("g_width");
	this->glsl_blending.addUniform("g_height");
	this->glsl_blending.addUniform("light_pos");
	this->glsl_blending.addUniform("view_pos");
	this->glsl_blending.addUniform("specular_flag");
	this->glsl_blending.addUniform("shininess");
	this->glsl_blending.disable();

	this->glsl_phong.enable();
	this->glsl_phong.addAttribute("position");
	this->glsl_phong.addAttribute("normal");
	this->glsl_phong.addUniform("MVP");
	this->glsl_phong.addUniform("light_pos");
	this->glsl_phong.addUniform("model_matrix");
	this->glsl_phong.addUniform("diffuse_reflectance");
	this->glsl_phong.addUniform("view_pos");
	this->glsl_phong.addUniform("light_ambient_color");
	this->glsl_phong.addUniform("light_diffuse_color");
	this->glsl_phong.addUniform("light_specular_color");
	this->glsl_phong.addUniform("gamma");
	this->glsl_phong.addUniform("shininess");
	this->glsl_phong.disable();
}

void model::display(glm::mat4 projection, glm::mat4 view, int g_width, int g_height, float shininess, bool scattering_model, bool change_light, bool model_center, bool specular_flag, glm::vec3 light_translation, glm::vec3 diffuse_comp, glm::vec3 ambient_comp, glm::vec3 specular_comp, glm::vec3 camera_position) {
	glm::mat4 projection_ortho, view_ortho, view_proj_ortho_light, view_proj_ortho_random, model_mat;
	glm::vec3 sigma_tr, center_model;

	projection_ortho = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 40.0f);

	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, scene_model->translation);
	model_mat = model_mat * glm::toMat4(scene_model->rotation);
	model_mat = glm::scale(model_mat, glm::vec3(scene_model->scale));
	center_model = glm::vec3(model_mat * glm::vec4(scene_model->center, 1.0f));

	if (scattering_model)
	{
		if (scene_model->change_values || change_light)
		{
			for (int i = 0; i < 2; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, light_buffers->g_buffer[i]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glsl_g_buffer.enable();

				view_ortho = glm::lookAt(light_translation, center_model, glm::vec3(0.0f, 1.0f, 0.0f));
				view_proj_ortho_light = projection_ortho * view_ortho;

				glUniformMatrix4fv(glsl_g_buffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
				glUniformMatrix4fv(glsl_g_buffer.getLocation("vp_light"), 1, GL_FALSE, glm::value_ptr(view_proj_ortho_light));
				glUniform1i(glsl_g_buffer.getLocation("num_of_buffer"), i);

				glBindVertexArray(scene_model->vao);
				glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
				glBindVertexArray(0);
				glsl_g_buffer.disable();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			glsl_scattered_map.enable();
			glBindFramebuffer(GL_FRAMEBUFFER, scattered_maps->buffer);
			glStencilFunc(GL_ALWAYS, 1, -1);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			materials->materials[scene_model->current_material]->precalculate_values(scene_model->asymmetry_param_g);
			sigma_tr = materials->materials[scene_model->current_material]->effective_transport_coeff;
			halton_generator->generate_samples(min(min(sigma_tr.x, sigma_tr.y), sigma_tr.z) / scene_model->q, scene_model->radius, num_of_samples_per_frag);

			std::vector<glm::mat4> view_proj_ortho_randoms;

			for (size_t j = 0; j < num_of_ortho_cameras; j++)
			{
				view_ortho = glm::lookAt(halton_generator->camera_positions[j], center_model, glm::vec3(0.0f, 1.0f, 0.0f));
				view_proj_ortho_random = projection_ortho * view_ortho;
				view_proj_ortho_randoms.push_back(view_proj_ortho_random);
			}

			view_ortho = glm::lookAt(light_translation, center_model, glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_light = projection_ortho * view_ortho;

			glUniformMatrix4fv(glsl_scattered_map.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
			glUniform1i(glsl_scattered_map.getLocation("n_cameras"), num_of_ortho_cameras);
			glUniformMatrix4fv(glsl_scattered_map.getLocation("cameras_matrix"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(view_proj_ortho_randoms[0]));
			glUniformMatrix4fv(glsl_scattered_map.getLocation("vp_light"), 1, GL_FALSE, glm::value_ptr(view_proj_ortho_light));
			glUniform1i(glsl_scattered_map.getLocation("g_position"), 0);
			glUniform1i(glsl_scattered_map.getLocation("g_normal"), 1);
			glUniform1i(glsl_scattered_map.getLocation("g_depth"), 2);
			glUniform1f(glsl_scattered_map.getLocation("radius"), scene_model->radius);
			glUniform1f(glsl_scattered_map.getLocation("bias"), scene_model->bias);
			if (model_center)
				glUniform3fv(glsl_scattered_map.getLocation("model_center"), 1, glm::value_ptr(center_model));
			else
				glUniform3fv(glsl_scattered_map.getLocation("model_center"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
			glUniform1i(glsl_scattered_map.getLocation("n_samples"), num_of_samples_per_frag);
			glUniform2fv(glsl_scattered_map.getLocation("samples"), num_of_samples_per_frag, glm::value_ptr(halton_generator->samples[0]));
			glUniform1f(glsl_scattered_map.getLocation("asymmetry_param_g"), scene_model->asymmetry_param_g);
			glUniform1f(glsl_scattered_map.getLocation("refractive_index"), scene_model->refractive_index);
			glUniform3fv(glsl_scattered_map.getLocation("diffuse_reflectance"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->diffuse_reflectance));
			glUniform3fv(glsl_scattered_map.getLocation("light_pos"), 1, glm::value_ptr(light_translation));
			glUniform3fv(glsl_scattered_map.getLocation("light_diffuse_color"), 1, glm::value_ptr(diffuse_comp));
			glUniform1i(glsl_scattered_map.getLocation("g_width"), g_width);
			glUniform1i(glsl_scattered_map.getLocation("g_height"), g_height);

			// Valores pre-calculados
			glUniform3fv(glsl_scattered_map.getLocation("attenuation_coeff"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->attenuation_coeff));
			glUniform3fv(glsl_scattered_map.getLocation("D"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->D));
			glUniform3fv(glsl_scattered_map.getLocation("effective_transport_coeff"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->effective_transport_coeff));
			glUniform1f(glsl_scattered_map.getLocation("c_phi_1"), materials->materials[scene_model->current_material]->c_phi_1);
			glUniform1f(glsl_scattered_map.getLocation("c_phi_2"), materials->materials[scene_model->current_material]->c_phi_2);
			glUniform1f(glsl_scattered_map.getLocation("c_e"), materials->materials[scene_model->current_material]->c_e);
			glUniform1f(glsl_scattered_map.getLocation("A"), materials->materials[scene_model->current_material]->A);
			glUniform3fv(glsl_scattered_map.getLocation("de"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->de));
			glUniform3fv(glsl_scattered_map.getLocation("zr"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->zr));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_position);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_normal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_depth);

			glBindVertexArray(scene_model->vao);
			glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
			glBindVertexArray(0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glsl_scattered_map.disable();
			scene_model->change_values = false;
		}
	}

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glStencilFunc(GL_ALWAYS, 1, -1);
	if (scattering_model)
	{
		glsl_blending.enable();

		glUniform3f(glsl_blending.getLocation("camera_pos"), camera_position[0], camera_position[1], camera_position[2]);

		std::vector<glm::mat4> view_proj_ortho_randoms;
		std::vector<glm::vec3> cameras_dirs;

		for (size_t j = 0; j < num_of_ortho_cameras; j++)
		{
			view_ortho = glm::lookAt(halton_generator->camera_positions[j], center_model, glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_random = projection_ortho * view_ortho;
			view_proj_ortho_randoms.push_back(view_proj_ortho_random);
			cameras_dirs.push_back(halton_generator->camera_positions[j]);
		}

		glUniformMatrix4fv(glsl_blending.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glsl_blending.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model_mat));
		glUniform1i(glsl_blending.getLocation("scattered_map"), 0);
		glUniform1i(glsl_blending.getLocation("depth_map"), 1);
		glUniform1f(glsl_blending.getLocation("epsilon"), scene_model->epsilon);
		glUniform1f(glsl_blending.getLocation("refractive_index"), scene_model->refractive_index);
		glUniform1i(glsl_blending.getLocation("n_cameras"), num_of_ortho_cameras);
		glUniformMatrix4fv(glsl_blending.getLocation("cameras_matrix"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(view_proj_ortho_randoms[0]));
		glUniformMatrix4fv(glsl_blending.getLocation("cameras_dirs"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(cameras_dirs[0]));
		glUniform1f(glsl_blending.getLocation("gamma"), scene_model->gamma);
		glUniform1f(glsl_blending.getLocation("bias"), scene_model->bias);
		glUniform1i(glsl_blending.getLocation("g_width"), g_width);
		glUniform1i(glsl_blending.getLocation("g_height"), g_height);
		glUniform3fv(glsl_blending.getLocation("light_pos"), 1, glm::value_ptr(light_translation));
		glUniform3fv(glsl_blending.getLocation("view_pos"), 1, glm::value_ptr(camera_position));
		glUniform1i(glsl_blending.getLocation("specular_flag"), specular_flag);
		glUniform1f(glsl_blending.getLocation("shininess"), shininess);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, scattered_maps->array_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, scattered_maps->depth_texture);

		glBindVertexArray(scene_model->vao);
		glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
		glBindVertexArray(0);
		glsl_blending.disable();
	}
	else
	{
		glsl_phong.enable();
		glUniformMatrix4fv(glsl_phong.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glsl_phong.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model_mat));
		glUniform3fv(glsl_phong.getLocation("diffuse_reflectance"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->diffuse_reflectance));
		glUniform3fv(glsl_phong.getLocation("light_pos"), 1, glm::value_ptr(light_translation));
		glUniform3fv(glsl_phong.getLocation("view_pos"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glsl_phong.getLocation("light_ambient_color"), 1, glm::value_ptr(ambient_comp));
		glUniform3fv(glsl_phong.getLocation("light_diffuse_color"), 1, glm::value_ptr(diffuse_comp));
		glUniform3fv(glsl_phong.getLocation("light_specular_color"), 1, glm::value_ptr(specular_comp));
		glUniform1f(glsl_phong.getLocation("gamma"), scene_model->gamma);
		glUniform1f(glsl_phong.getLocation("shininess"), shininess);

		glBindVertexArray(scene_model->vao);
		glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
		glBindVertexArray(0);
		glsl_phong.disable();
	}
}

void model::render_quad() {
	if (this->quad_vao == 0)
	{
		GLfloat quad_vertices[] =
		{
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		glGenVertexArrays(1, &this->quad_vao);
		glGenBuffers(1, &this->quad_vbo);
		glBindVertexArray(this->quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, this->quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

	glBindVertexArray(this->quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void model::display_g_buffer(texture_t current_texture_type) {
	glm::mat4 model_mat;

	this->glsl_g_buffer_plane.enable();
	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, glm::vec3(0.7, -0.7, -1.0));
	model_mat = glm::scale(model_mat, glm::vec3(0.3f));
	glUniformMatrix4fv(this->glsl_g_buffer_plane.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniform1i(this->glsl_g_buffer_plane.getLocation("camera_select"), this->selected_camera);
	glUniform1i(this->glsl_g_buffer_plane.getLocation("quad_texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	if (current_texture_type == Scattered_Map)
		glBindTexture(GL_TEXTURE_2D_ARRAY, this->scattered_maps->array_texture);
	else if (current_texture_type == GBuffer_Light_Position)
		glBindTexture(GL_TEXTURE_2D_ARRAY, this->light_buffers->g_position);
	else if (current_texture_type == GBuffer_Light_Normal)
		glBindTexture(GL_TEXTURE_2D_ARRAY, this->light_buffers->g_normal);
	else if (current_texture_type == GBuffer_Light_Depth)
		glBindTexture(GL_TEXTURE_2D_ARRAY, this->light_buffers->g_depth);
	this->render_quad();
	this->glsl_g_buffer_plane.disable();
}