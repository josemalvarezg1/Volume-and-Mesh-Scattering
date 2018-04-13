#include "Model.h"

mesh::mesh()
{
	this->translation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->scale = 5.0f;
	this->ambient_comp = glm::vec3(0.15f, 0.15f, 0.15f);
	this->diffuse_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->specular_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->asymmetry_param_g = 0.77f;
	this->refractive_index = 1.3f;
	this->current_material = Crema;
	this->q = 1.0f;
	this->radius = 0.1f;
	this->epsilon = 0.02f;
	this->gamma = 1.0f;
	this->change_values = true;
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

mesh_set::mesh_set()
{
	this->visible_interface = false;
	this->model_interface = interface_model::instance();
}

mesh_set::~mesh_set()
{
	this->mesh_models.empty();
}

void mesh_set::click_model(int selected_model)
{
	this->model_interface->show();
	this->model_interface->translation = this->mesh_models[selected_model]->translation;
	this->model_interface->rotation = this->mesh_models[selected_model]->rotation;
	this->model_interface->scale = this->mesh_models[selected_model]->scale;
	this->model_interface->asymmetry_param_g = this->mesh_models[selected_model]->asymmetry_param_g;
	this->model_interface->current_material = this->mesh_models[selected_model]->current_material;
	this->model_interface->q = this->mesh_models[selected_model]->q;
	this->model_interface->radius = this->mesh_models[selected_model]->radius;
	this->model_interface->gamma = this->mesh_models[selected_model]->gamma;
	this->model_interface->epsilon = this->mesh_models[selected_model]->epsilon;
	this->visible_interface = true;
}

void mesh_set::not_click_model()
{
	this->model_interface->hide();
	this->visible_interface = false;
}

void mesh_set::update_interface(int selected_model)
{
	if (visible_interface && selected_model >= 0)
	{
		if (this->mesh_models[selected_model]->translation != this->model_interface->translation)
		{
			this->mesh_models[selected_model]->translation = this->model_interface->translation;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->rotation != this->model_interface->rotation)
		{
			this->mesh_models[selected_model]->rotation = this->model_interface->rotation;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->scale != this->model_interface->scale)
		{
			this->mesh_models[selected_model]->scale = this->model_interface->scale;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->asymmetry_param_g != this->model_interface->asymmetry_param_g)
		{
			this->mesh_models[selected_model]->asymmetry_param_g = this->model_interface->asymmetry_param_g;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->radius != this->model_interface->radius)
		{
			this->mesh_models[selected_model]->radius = this->model_interface->radius;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->epsilon != this->model_interface->epsilon)
		{
			this->mesh_models[selected_model]->epsilon = this->model_interface->epsilon;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->gamma != this->model_interface->gamma)
		{
			this->mesh_models[selected_model]->gamma = this->model_interface->gamma;
			this->mesh_models[selected_model]->change_values = true;
		}
		if (this->mesh_models[selected_model]->current_material != this->model_interface->current_material)
		{
			this->mesh_models[selected_model]->current_material = this->model_interface->current_material;
			this->mesh_models[selected_model]->change_values = true;
		}
	}
}