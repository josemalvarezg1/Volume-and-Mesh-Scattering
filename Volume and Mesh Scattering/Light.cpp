#include "Light.h"

light::light()
{
	this->translation = glm::vec3(1.0f, 2.0f, 1.0f);
	this->ambientComp = glm::vec3(0.15f, 0.15f, 0.15f);
	this->diffuseComp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->specularComp = glm::vec3(1.0f, 1.0f, 1.0f);
	this->on = false;
	this->viewInterface = false;
	this->load("Models/obj/light.obj");
	this->initShaders();
}

light::~light()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void light::initShaders()
{
	this->lightP.loadShader("Shaders/light.vert", CGLSLProgram::VERTEX);
	this->lightP.loadShader("Shaders/light.frag", CGLSLProgram::FRAGMENT);

	this->lightP.create_link();

	this->lightP.enable();
	this->lightP.addAttribute("vertexCoords");
	this->lightP.addUniform("MVP");
	this->lightP.addUniform("color");
	this->lightP.disable();
}

void light::createVBO()
{
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertex.size() * sizeof(glm::vec3), &this->vertex[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glGenBuffers(1, &this->vindex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vindex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->index.size() * sizeof(GLuint), &this->index[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void light::load(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in);
	if (file.is_open())
	{
		GLfloat x, y, z, maxAxis;
		std::string buffer, first, second, third;
		glm::vec3 auxVertex, center;
		unsigned int i;
		while (!file.eof())
		{
			file >> buffer;
			if (buffer == "v")
			{
				file >> x >> y >> z;
				auxVertex = glm::vec3(x, y, z);
				this->vertex.push_back(auxVertex);
			}
			else
			{
				if (buffer == "f")
				{
					file >> first >> second >> third;
					this->insertIndex(first);
					this->insertIndex(second);
					this->insertIndex(third);
				}
			}
		}
		file.close();
		this->maximus.x = vertex[0].x;
		this->minimous.x = vertex[0].x;
		this->maximus.y = vertex[0].y;
		this->minimous.y = vertex[0].y;
		this->maximus.z = vertex[0].y;
		this->minimous.z = vertex[0].y;
		for (i = 0; i < this->vertex.size(); i++)
		{
			if (this->vertex[i].x > this->maximus.x) this->maximus.x = this->vertex[i].x;
			if (this->vertex[i].x < this->minimous.x) this->minimous.x = this->vertex[i].x;
			if (this->vertex[i].y > this->maximus.y) this->maximus.y = this->vertex[i].y;
			if (this->vertex[i].y < this->minimous.y) this->minimous.y = this->vertex[i].y;
			if (this->vertex[i].z > this->maximus.z) this->maximus.z = this->vertex[i].z;
			if (this->vertex[i].z < this->minimous.z) this->minimous.z = this->vertex[i].z;
		}
		center = glm::vec3((this->maximus.x + this->minimous.x) / 2.0f, (this->maximus.y + this->minimous.y) / 2.0f, (this->maximus.z + this->minimous.z) / 2.0f);
		maxAxis = this->maximus.x;
		for (i = 0; i < 3u; i++) if (maxAxis < this->maximus[i]) maxAxis = this->maximus[i];
		for (i = 0; i < this->vertex.size(); i++) this->vertex[i] = (this->vertex[i] - center) / maxAxis;
		this->maximus.x = (this->maximus.x - center.x) / maxAxis;
		this->minimous.x = (this->minimous.x - center.x) / maxAxis;
		this->maximus.y = (this->maximus.y - center.y) / maxAxis;
		this->minimous.y = (this->minimous.y - center.y) / maxAxis;
		this->maximus.z = (this->maximus.z - center.z) / maxAxis;
		this->minimous.z = (this->minimous.z - center.z) / maxAxis;
		this->createVBO();
	}
}

void light::insertIndex(std::string value)
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

bool light::intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, int screenWidth, int screenHeight)
{
	GLfloat ax, ay, tx1, tx2, tmin, tmax, amin, amax, ty1, ty2, tz1, tz2;
	glm::vec4 cameraPosition2, rayClip, rayEye, min, max;
	glm::vec3 rayWorld;
	glm::mat4 model;

	ax = (GLfloat)((2.0f * x) / screenWidth - 1.0f);
	ay = (GLfloat)(1.0f - (2.0f * y) / screenHeight);

	rayClip = glm::vec4(ax, ay, -1.0f, 1.0f);
	rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
	rayEye = glm::inverse(view) * rayEye;

	model = glm::translate(glm::mat4(1.0f), this->translation) * glm::scale(model, glm::vec3(0.5f));
	rayWorld = glm::vec3(glm::normalize(glm::inverse(model) * glm::vec4(rayEye.x, rayEye.y, rayEye.z, 0.0f)));
	cameraPosition2 = glm::inverse(model) * glm::vec4(cameraPosition, 1.0f);

	min = glm::vec4(this->minimous, 1.0f);
	max = glm::vec4(this->maximus, 1.0f);

	rayWorld = glm::vec3(1.0 / rayWorld.x, 1.0 / rayWorld.y, 1.0 / rayWorld.z);

	tx1 = (min.x - cameraPosition2.x) * rayWorld.x;
	tx2 = (max.x - cameraPosition2.x) * rayWorld.x;
	tmin = glm::min(tx1, tx2);
	tmax = glm::max(tx1, tx2);
	amin = tmin, amax = tmax;

	ty1 = (min.y - cameraPosition2.y) * rayWorld.y;
	ty2 = (max.y - cameraPosition2.y) * rayWorld.y;
	amin = glm::min(ty1, ty2);
	tmin = glm::max(amin, tmin);
	amax = glm::max(ty1, ty2);
	tmax = glm::min(tmax, amax);

	tz1 = (min.z - cameraPosition2.z) * rayWorld.z;
	tz2 = (max.z - cameraPosition2.z) * rayWorld.z;
	amin = glm::min(tz1, tz2);
	tmin = glm::max(amin, tmin);
	amax = glm::max(tz1, tz2);
	tmax = glm::min(tmax, amax);

	if (tmax >= tmin && tmin < FLT_MAX) return true;
	return false;
}
//
//bool light::clickLight(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, int screenWidth, int screenHeight)
//{
//
//	if (this->intersection(x, y, projection, view, cameraPosition, screenWidth, screenHeight))
//	{
//		this->lightInterface->show();
//		this->lightInterface->on = this->on;
//		this->lightInterface->translation = this->translation;
//		this->lightInterface->ambientComp = this->ambientComp;
//		this->lightInterface->diffuseComp = this->diffuseComp;
//		this->lightInterface->specularComp = this->specularComp;
//		this->viewInterface = true;
//		return true;
//	}
//	this->lightInterface->hide();
//	this->viewInterface = false;
//	return false;
//}

void light::display(glm::mat4 &viewProjection)
{
	glm::mat4 model, MVP;
	model = glm::translate(glm::mat4(1.0f), this->translation) * glm::scale(model, glm::vec3(0.25f));
	MVP = viewProjection * model;

	this->lightP.enable();
	glUniformMatrix4fv(this->lightP.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(this->lightP.getLocation("color"), 1, &diffuseComp[0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vindex);
	glDrawElements(GL_TRIANGLES, this->index.size(), GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	this->lightP.disable();
}

void light::onOffLight()
{
	if (this->on)
		this->on = false;
	else
		this->on = true;
}

void light::updateInterface()
{
	if (viewInterface)
	{
		/*this->translation = this->lightInterface->translation;
		this->ambientComp = this->lightInterface->ambientComp;
		this->diffuseComp = this->lightInterface->diffuseComp;
		this->specularComp = this->lightInterface->specularComp;
		this->on = this->lightInterface->on;*/
	}
}