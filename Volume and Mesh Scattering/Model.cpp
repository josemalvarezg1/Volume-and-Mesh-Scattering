#include "Model.h"

std::vector<std::string> split(std::string s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (getline(ss, item, delim))
		tokens.push_back(item);
	return tokens;
}

std::vector<std::string> process_attribute(std::string value)
{
	std::string output;
	unique_copy(value.begin(), value.end(), std::back_insert_iterator<std::string>(output), [](char a, char b) { return isspace(a) && isspace(b); });
	return split(output.erase(0, 1), ' ');
}

model::model()
{
	this->translation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	this->scale = 5.0f;
	this->ambient_comp = glm::vec3(0.15f, 0.15f, 0.15f);
	this->diffuse_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->specular_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->shininess = 128.0f;
}

void model::create_vbo()
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

void model::set_max_min_value(GLfloat x, GLfloat y, GLfloat z)
{
	if (x > this->max_vertex.x) this->max_vertex.x = x; if (y > this->max_vertex.y) this->max_vertex.y = y; if (z > this->max_vertex.z) this->max_vertex.z = z;
	if (x < this->min_vertex.x) this->min_vertex.x = x; if (y < this->min_vertex.y) this->min_vertex.y = y; if (z < this->min_vertex.z) this->min_vertex.z = z;
}

void model::calculate_center()
{
	this->center.x = (this->max_vertex.x + this->min_vertex.x) / 2.0f;
	this->center.y = (this->max_vertex.y + this->min_vertex.y) / 2.0f;
	this->center.z = (this->max_vertex.z + this->min_vertex.z) / 2.0f;
	this->max_value = this->max_vertex.x;
	for (unsigned int i = 0; i < 3; i++) if (this->max_value < this->max_vertex[i]) this->max_value = this->max_vertex[i];
}

void model::triangulate(std::vector<unsigned int> aux_index_vertices, std::vector<unsigned int> aux_index_normals, std::vector<glm::uvec3> &index_vertices, std::vector<glm::uvec3> &index_normals)
{
	for (unsigned int i = 1; (i + 1) < aux_index_vertices.size(); i++)
	{
		index_vertices.push_back(glm::uvec3(aux_index_vertices[0], aux_index_vertices[i], aux_index_vertices[i + 1]));
		index_normals.push_back(glm::uvec3(aux_index_normals[0], aux_index_normals[i], aux_index_normals[i + 1]));
	}
}

void model::load(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in);
	std::vector<glm::vec3>  aux_vertices, aux_normals;
	std::vector<unsigned int> aux_index_vertices, aux_index_normals;
	std::vector<glm::uvec3> index_vertices, index_normals;

	if (file.is_open())
	{
		std::string buffer;
		bool vertex_normals;
		float x, y, z;
		int num_of_index;
		glm::vec3 aux_element;

		while (!file.eof())
		{
			file >> buffer;
			if (buffer == "v")
			{
				file >> x >> y >> z;
				aux_element = glm::vec3(x, y, z);
				aux_vertices.push_back(aux_element);
				set_max_min_value(x, y, z);
			}
			else if (buffer == "vn")
			{
				file >> x >> y >> z;
				aux_element = glm::vec3(x, y, z);
				aux_normals.push_back(aux_element);
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
					triangulate(aux_index_vertices, aux_index_normals, index_vertices, index_normals);
				}
			}
		}

		file.close();

		glm::uvec3 vertex_aux_index, normal_aux_index;
		num_of_index = index_vertices.size();
		vertices = std::vector<glm::vec3>(num_of_index * 3);
		normals = std::vector<glm::vec3>(num_of_index * 3);
		calculate_center();

		for (size_t i = 0; i < num_of_index; i++)
		{
			vertex_aux_index = index_vertices[i];
			vertices[(i * 3)] = (aux_vertices[vertex_aux_index.x] - center) / max_value;
			vertices[(i * 3) + 1] = (aux_vertices[vertex_aux_index.y] - center) / max_value;
			vertices[(i * 3) + 2] = (aux_vertices[vertex_aux_index.z] - center) / max_value;
		
			normal_aux_index = index_normals[i];
			normals[(i * 3)] = aux_normals[normal_aux_index.x];
			normals[(i * 3) + 1] = aux_normals[normal_aux_index.y];
			normals[(i * 3) + 2] = aux_normals[normal_aux_index.z];
		}

		max_vertex = (max_vertex - center) / max_value;
		min_vertex = (min_vertex - center) / max_value;
	}
	create_vbo();
}


//Para la rotación (autorotación)
void model::setQuat(const float *eje, float angulo, float *quat) {
	float sina2, norm;
	sina2 = (float)sin(0.5f * angulo);
	norm = (float)sqrt(eje[0] * eje[0] + eje[1] * eje[1] + eje[2] * eje[2]);
	quat[0] = sina2 * eje[0] / norm;
	quat[1] = sina2 * eje[1] / norm;
	quat[2] = sina2 * eje[2] / norm;
	quat[3] = (float)cos(0.5f * angulo);
}

//Para la rotación (autorotación)
void model::multiplicarQuat(const float *q1, const float *q2, float *qout) {
	float qr[4];
	qr[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	qr[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	qr[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	qr[3] = q1[3] * q2[3] - (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
	qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

//Para crear la matriz de escalamiento
glm::mat4 model::scale_en_matriz(float scale_tx) {
	glm::mat4 scaleMatrix = glm::mat4(glm::vec4(scale_tx, 0.0, 0.0, 0.0), glm::vec4(0.0, scale_tx, 0.0, 0.0), glm::vec4(0.0, 0.0, scale_tx, 0.0), glm::vec4(0.0, 0.0, 0.0, 1)); //Creo matriz de escalamiento
	return scaleMatrix;
}

//Para crear la matriz de translate
glm::mat4 model::translate_en_matriz(float translate_tx, float translate_ty, float translate_tz) {
	glm::mat4 translateMatrix = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(translate_tx, translate_ty, translate_tz, 1)); //Creo matriz de translate
	return translateMatrix;
}

//Para crear la matriz de rotación
glm::mat4 model::rotacion_en_matriz(float rotacion_tx, float rotacion_ty, float rotacion_tz, float rotacion_ta) {
	glm::mat4 rotateMatrix;
	//Creo matriz de rotación usando los quat
	rotateMatrix[0][0] = 1.0 - 2.0 * (rotacion_ty * rotacion_ty + rotacion_tz * rotacion_tz);
	rotateMatrix[0][1] = 2.0 * (rotacion_tx * rotacion_ty + rotacion_tz * rotacion_ta);
	rotateMatrix[0][2] = 2.0 * (rotacion_tx * rotacion_tz - rotacion_ty * rotacion_ta);
	rotateMatrix[0][3] = 0.0;
	rotateMatrix[1][0] = 2.0 * (rotacion_tx * rotacion_ty - rotacion_tz * rotacion_ta);
	rotateMatrix[1][1] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_tz * rotacion_tz);
	rotateMatrix[1][2] = 2.0 * (rotacion_ty * rotacion_tz + rotacion_tx * rotacion_ta);
	rotateMatrix[1][3] = 0.0;
	rotateMatrix[2][0] = 2.0 * (rotacion_tx * rotacion_tz + rotacion_ty * rotacion_ta);
	rotateMatrix[2][1] = 2.0 * (rotacion_ty * rotacion_tz - rotacion_tx * rotacion_ta);
	rotateMatrix[2][2] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_ty * rotacion_ty);
	rotateMatrix[2][3] = 0.0;
	rotateMatrix[3][0] = 0.0;
	rotateMatrix[3][1] = 0.0;
	rotateMatrix[3][2] = 0.0;
	rotateMatrix[3][3] = 1.0;
	return rotateMatrix;
}