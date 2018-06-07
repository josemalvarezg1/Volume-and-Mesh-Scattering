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
	this->radius = 0.01f;
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
	this->max_value = this->max_vertex.x;
	for (unsigned int i = 0; i < 3; i++) if (this->max_value < this->max_vertex[i]) this->max_value = this->max_vertex[i];
}

void mesh::load(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in);
	if (file.is_open())
	{
		std::string buffer;
		std::vector<glm::vec3>  aux_vertices, aux_normals;
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

					for (int i = 1; (i + 1) < aux_index_vertices.size(); i++)
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

		for (int i = 0; i < index_vertices.size(); i++)
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

		aux_index_vertices.clear();
		aux_index_normals.clear();
		index_vertices.clear();
		index_normals.clear();
		aux_vertices.clear();
		aux_normals.clear();

		this->max_vertex = (this->max_vertex - this->center) / this->max_value;
		this->min_vertex = (this->min_vertex - this->center) / this->max_value;

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