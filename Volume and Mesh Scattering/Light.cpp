#include "Light.h"

light::light()
{
	this->on = false;
	this->visible_interface = false;
	this->translation = glm::vec3(-3.0f, 3.0f, 5.0f);
	this->ambient_comp = glm::vec3(0.15f, 0.15f, 0.15f);
	this->diffuse_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->specular_comp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->light_interface = interface_light::instance();

	this->init_shaders();
	this->load("Models/obj/light.obj");
}

light::~light()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void light::init_shaders()
{
	this->light_program.loadShader("Shaders/light.vert", CGLSLProgram::VERTEX);
	this->light_program.loadShader("Shaders/light.frag", CGLSLProgram::FRAGMENT);

	this->light_program.create_link();

	this->light_program.enable();
	this->light_program.addAttribute("position");
	this->light_program.addUniform("mvp");
	this->light_program.addUniform("diffuse_color");
	this->light_program.disable();
}

void light::set_max_min_value(GLfloat x, GLfloat y, GLfloat z)
{
	if (x > this->max_vertex.x) this->max_vertex.x = x; if (y > this->max_vertex.y) this->max_vertex.y = y; if (z > this->max_vertex.z) this->max_vertex.z = z;
	if (x < this->min_vertex.x) this->min_vertex.x = x; if (y < this->min_vertex.y) this->min_vertex.y = y; if (z < this->min_vertex.z) this->min_vertex.z = z;
}

void light::insert_index(std::string value)
{
	unsigned int foundOut, a;
	std::string nextString;
	foundOut = value.find('/', 0);
	if (foundOut != -1)
	{
		nextString = value.substr(0, foundOut);
		a = (unsigned int)(atoi(nextString.c_str()) - 1);
		this->index.push_back(a);
	}
}

void light::calculate_center()
{
	this->center.x = (this->max_vertex.x + this->min_vertex.x) / 2.0f;
	this->center.y = (this->max_vertex.y + this->min_vertex.y) / 2.0f;
	this->center.z = (this->max_vertex.z + this->min_vertex.z) / 2.0f;
	this->max_value = this->max_vertex.x;
	for (unsigned int i = 0; i < 3; i++) if (this->max_value < this->max_vertex[i]) this->max_value = this->max_vertex[i];
}

void light::load(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in);
	if (file.is_open())
	{
		GLfloat x, y, z;
		std::string buffer, first, second, third;
		while (!file.eof())
		{
			file >> buffer;
			if (buffer == "v")
			{
				file >> x >> y >> z;
				this->vertices.push_back(glm::vec3(x, y, z));
				set_max_min_value(x, y, z);
			}
			else if (buffer == "f")
			{
				file >> first >> second >> third;
				this->insert_index(first);
				this->insert_index(second);
				this->insert_index(third);
			}
		}
		file.close();
		calculate_center();
		for (int i = 0; i < this->vertices.size(); i++) this->vertices[i] = (this->vertices[i] - this->center) / this->max_value;
		max_vertex = (max_vertex - center) / max_value;
		min_vertex = (min_vertex - center) / max_value;
		this->create_vbo();
	}
}

void light::create_vbo()
{
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), &this->vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glGenBuffers(1, &this->vindex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vindex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->index.size() * sizeof(GLuint), &this->index[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void light::display(glm::mat4 &view_projection)
{
	glm::mat4 model, mvp;
	model = glm::translate(glm::mat4(1.0f), this->translation) * glm::scale(model, glm::vec3(0.20f));
	mvp = view_projection * model;

	this->light_program.enable();
	glUniformMatrix4fv(this->light_program.getLocation("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
	glUniform3fv(this->light_program.getLocation("diffuse_color"), 1, &diffuse_comp[0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vindex);
	glDrawElements(GL_TRIANGLES, this->index.size(), GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	this->light_program.disable();
}

void light::click_light()
{
	this->light_interface->show();
	this->light_interface->on = this->on;
	this->light_interface->translation = this->translation;
	this->light_interface->ambient_comp = this->ambient_comp;
	this->light_interface->diffuse_comp = this->diffuse_comp;
	this->light_interface->specular_comp = this->specular_comp;
	this->visible_interface = true;
}

void light::not_click_light()
{
	this->light_interface->hide();
	this->visible_interface = false;
}

void light::on_off_light()
{
	if (this->on)
		this->on = false;
	else
		this->on = true;
}

bool light::update_interface()
{
	if (visible_interface)
	{				
		this->ambient_comp = this->light_interface->ambient_comp;
		this->specular_comp = this->light_interface->specular_comp;
		this->on = this->light_interface->on;
		if (this->translation != this->light_interface->translation) 
		{			
			this->translation = this->light_interface->translation;
			return true;
		}
		if (this->diffuse_comp != this->light_interface->diffuse_comp)
		{
			this->diffuse_comp = this->light_interface->diffuse_comp;
			return true;
		}
	}
	return false;
}