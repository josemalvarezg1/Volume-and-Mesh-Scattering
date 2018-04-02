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
	static GLfloat vertexCube[] =
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

	glGenVertexArrays(1, &this->vaoCube);
	glBindVertexArray(this->vaoCube);

	glGenBuffers(1, &this->vboCube);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboCube);
	glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat), vertexCube, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(sizeof(GL_FLOAT) * 3));

	glGenBuffers(1, &this->eboCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->eboCube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 14 * sizeof(GLuint), elements, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

cube::~cube()
{
	glDeleteVertexArrays(1, &this->vaoCube);
	glDeleteBuffers(1, &this->vboCube);
	glDeleteBuffers(1, &this->eboCube);
}

void cube::display()
{
	glBindVertexArray(this->vaoCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->eboCube);
	glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

volume::volume(std::string path, GLuint width, GLuint height, GLuint depth, GLuint numOfBits)
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
	this->numOfBits = numOfBits;
	this->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->translation = glm::vec3(0.0f);
	this->escalation = 1.0f;
	this->step = (GLfloat)(1.0f / sqrt((this->width * this->width) + (this->height * this->height) + (this->depth * this->depth)));
	if (this->numOfBits == 8u)
		if (length == this->width * this->height * this->depth)
		{
			char *textureData;
			textureData = new char[length];
			file.seekg(0, file.beg);
			file.read((char*)textureData, length);
			file.close();
			glGenTextures(1, &this->volumeText);
			glBindTexture(GL_TEXTURE_3D, this->volumeText);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, this->width, this->height, this->depth, 0, GL_RED, GL_UNSIGNED_BYTE, textureData);
			glBindTexture(GL_TEXTURE_3D, 0);
			delete[] textureData;
		}
		else
		{
			std::cout << "Error: dimensiones erroneas" << std::endl;
			return;
		}
	else
		if (length == this->width * this->height * this->depth * 2u)
		{
			short *textureData;
			textureData = new short[length];
			file.seekg(0, file.beg);
			file.read((char*)textureData, length);
			file.close();
			glGenTextures(1, &this->volumeText);
			glBindTexture(GL_TEXTURE_3D, this->volumeText);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, this->width, this->height, this->depth, 0, GL_RED, GL_UNSIGNED_SHORT, textureData);
			delete[] textureData;
		}
		else
		{
			std::cout << "Error: dimensiones erroneas" << std::endl;
			return;
		}
}

volume::~volume()
{
	glDeleteTextures(1, &this->volumeText);
}

volumeRender::volumeRender(int screenWidth, int screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->initShaders();
	this->unitaryCube = new cube();
	this->indexSelect = -1;
	this->pressVolumeRight = false;
	this->pressVolumeLeft = false;
	glGenTextures(1, &this->transferFunctionText);
	glGenTextures(1, &this->backFaceText);
	glGenFramebuffers(1, &this->frameBuffer);
	this->createBackFaceText();
	this->createFrameBuffer();
	this->visible_interface = false;
	this->volume_interface = interface_volume::instance();
}

volumeRender::~volumeRender()
{
	glDeleteTextures(1, &this->backFaceText);
	glDeleteTextures(1, &this->transferFunctionText);
	glDeleteFramebuffers(1, &this->frameBuffer);
	this->unitaryCube->~cube();
	for (unsigned int i = 0; i < this->volumes.size(); ++i) this->volumes[i]->~volume();
}

void volumeRender::dropPath(int count, const char** paths)
{
	glm::uvec4 parameters;
	for (int i = 0; i < count; i++)
	{
		std::string path(paths[i]);
		if (this->processPath(path))
		{
			parameters = this->getParameters(path);
			if (parameters != glm::uvec4(0))
			{
				volume *newVolume = new volume(path, parameters[0], parameters[1], parameters[2], parameters[3]);
				this->volumes.push_back(newVolume);
				this->indexSelect = this->volumes.size() - 1;
			}
			else
				std::cout << "Error cargando volumen" << std::endl;
		}
	}
}

bool volumeRender::processPath(std::string path)
{
	std::string newPath;
	newPath = path.substr(path.find_last_of('\\')).erase(0, 1);
	newPath = newPath.substr(0, newPath.find(".raw"));
	for (unsigned int i = 0; i < this->volumes.size(); i++)
		if (newPath.compare(this->volumes[i]->name) == 0)
			return false;
	return true;
}

glm::uvec4 volumeRender::getParameters(std::string path)
{
	std::string newPath;
	std::vector<std::string> parameters, dimensions;
	newPath = path.substr(path.find_last_of('\\')).erase(0, 1);
	newPath = newPath.substr(0, newPath.find(".raw"));
	parameters = split(newPath, '_');
	if (parameters.size() == 3)
	{
		dimensions = split(parameters[1], 'x');
		if (dimensions.size() == 3)
			return glm::uvec4(std::stoi(dimensions[0]), std::stoi(dimensions[1]), std::stoi(dimensions[2]), std::stoi(parameters[2]));
	}
	return glm::uvec4(0);
}

void volumeRender::initShaders()
{
	this->backface.loadShader("Shaders/backFace.vert", CGLSLProgram::VERTEX);
	this->backface.loadShader("Shaders/backFace.frag", CGLSLProgram::FRAGMENT);
	this->raycasting.loadShader("Shaders/rayCasting.vert", CGLSLProgram::VERTEX);
	this->raycasting.loadShader("Shaders/rayCasting.frag", CGLSLProgram::FRAGMENT);

	this->backface.create_link();
	this->raycasting.create_link();

	this->backface.enable();
	this->backface.addAttribute("vertexCoords");
	this->backface.addAttribute("volumeCoords");
	this->backface.addUniform("MVP");
	this->backface.disable();

	this->raycasting.enable();
	this->raycasting.addAttribute("vertexCoords");
	this->raycasting.addAttribute("volumeCoords");
	this->raycasting.addUniform("MVP");
	this->raycasting.addUniform("model");
	this->raycasting.addUniform("screenSize");
	this->raycasting.addUniform("stepSize");
	this->raycasting.addUniform("lightPos");
	this->raycasting.addUniform("lighting");
	this->raycasting.addUniform("viewPos");
	this->raycasting.addUniform("ambientComp");
	this->raycasting.addUniform("diffuseComp");
	this->raycasting.addUniform("specularComp");
	this->raycasting.addUniform("backFaceText");
	this->raycasting.addUniform("volumeText");
	this->raycasting.addUniform("transferFunctionText");
	this->raycasting.disable();
}

void volumeRender::scrollVolume(double yoffset)
{
	if (this->indexSelect != -1)
	{
		if (yoffset == 1)
			if (this->volumes[this->indexSelect]->escalation <= 3.0) this->volumes[this->indexSelect]->escalation += 0.05f;
		if (yoffset == -1)
			if (this->volumes[this->indexSelect]->escalation >= 0.05)	this->volumes[this->indexSelect]->escalation -= 0.05f;
	}
}

bool volumeRender::clickVolume(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition, bool type)
{
	if (this->indexSelect != -1)
	{
		if (this->intersection(x, y, projection, view, cameraPosition))
		{
			if (type)
				this->pressVolumeRight = true;
			else
				this->pressVolumeLeft = true;
			this->xReference = x;
			this->yReference = y;

			this->volume_interface->show();
			this->volume_interface->translation = this->volumes[this->indexSelect]->translation;
			this->volume_interface->rotation = this->volumes[this->indexSelect]->rotation;
			this->volume_interface->scale = this->volumes[this->indexSelect]->escalation;
			this->visible_interface = true;

			return true;
		}
	}
	return false;
}

bool volumeRender::poscursorVolume(double x, double y)
{
	if (this->indexSelect != -1)
	{
		GLfloat offsetX, offsetY, angle, square;
		glm::quat quaternion;
		glm::vec3 axis;
		if (pressVolumeRight)
		{
			offsetX = (GLfloat)(x - this->xReference);
			offsetY = (GLfloat)(y - this->yReference);
			square = (offsetX * offsetX) + (offsetY * offsetY);
			angle = sqrt(square) * 0.15f;
			axis = glm::vec3(offsetY, offsetX, 0.0f);
			if (glm::length(axis) != 0.0f)
			{
				axis = glm::normalize(axis);
				quaternion = glm::angleAxis(angle, axis);
				this->volumes[this->indexSelect]->rotation = glm::cross(quaternion, this->volumes[this->indexSelect]->rotation);
			}
			this->xReference = x;
			this->yReference = y;
			return true;
		}
		if (pressVolumeLeft)
		{
			if (x > this->xReference) this->volumes[this->indexSelect]->translation.x += (GLfloat)(x - this->xReference) * 0.002f;
			else if (x < this->xReference) this->volumes[this->indexSelect]->translation.x -= (GLfloat)(this->xReference - x) * 0.002f;
			if (y > this->yReference) this->volumes[this->indexSelect]->translation.y -= (GLfloat)(y - this->yReference) * 0.002f;
			else if (y < this->yReference) this->volumes[this->indexSelect]->translation.y += (GLfloat)(this->yReference - y) * 0.002f;
			this->xReference = x;
			this->yReference = y;
			return true;
		}
	}
	return false;
}

void volumeRender::disableSelect()
{
	this->pressVolumeRight = false;
	this->pressVolumeLeft = false;
}

bool volumeRender::intersection(double x, double y, glm::mat4 &projection, glm::mat4 &view, glm::vec3 cameraPosition)
{
	GLfloat ax, ay, tx1, tx2, tmin, tmax, amin, amax, ty1, ty2, tz1, tz2;
	glm::vec4 cameraPosition2, rayClip, rayEye, min, max;
	glm::vec3 rayWorld;
	glm::mat4 model;

	ax = (GLfloat)((2.0f * x) / this->screenWidth - 1.0f);
	ay = (GLfloat)(1.0f - (2.0f * y) / this->screenHeight);

	rayClip = glm::vec4(ax, ay, -1.0f, 1.0f);
	rayEye = glm::inverse(projection) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
	rayEye = glm::inverse(view) * rayEye;

	model = glm::translate(glm::mat4(1.0f), this->volumes[this->indexSelect]->translation) * glm::mat4_cast(this->volumes[this->indexSelect]->rotation) * glm::scale(model, glm::vec3(this->volumes[this->indexSelect]->escalation));
	rayWorld = glm::vec3(glm::normalize(glm::inverse(model) * glm::vec4(rayEye.x, rayEye.y, rayEye.z, 0.0f)));
	cameraPosition2 = glm::inverse(model) * glm::vec4(cameraPosition, 1.0f);

	min = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
	max = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

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

void volumeRender::loadTransferFunct(GLfloat data[][4])
{
	glBindTexture(GL_TEXTURE_1D, this->transferFunctionText);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, data);
}

void volumeRender::UpdateTransferFunction(std::vector<double> points)
{
	GLfloat transFunctAux[256][4];
	GLfloat differenceR, differenceG, differenceB, differenceA;
	std::vector<unsigned int> index;
	unsigned int i, actual, j, differenceIndex;

	points[0] = 0.0;
	points[points.size() - 5] = 1.0;

	for (i = 0; i < points.size(); i += 5)
	{
		actual = (unsigned int)(points[i] * 255u);
		transFunctAux[actual][0] = GLfloat(points[i + 1]);
		transFunctAux[actual][1] = GLfloat(points[i + 2]);
		transFunctAux[actual][2] = GLfloat(points[i + 3]);
		transFunctAux[actual][3] = GLfloat(points[i + 4]);
		index.push_back(actual);
	}

	for (j = 0; j < (points.size() / 5) - 1; j++)
	{
		differenceR = transFunctAux[index[j + 1]][0] - transFunctAux[index[j]][0];
		differenceG = transFunctAux[index[j + 1]][1] - transFunctAux[index[j]][1];
		differenceB = transFunctAux[index[j + 1]][2] - transFunctAux[index[j]][2];
		differenceA = transFunctAux[index[j + 1]][3] - transFunctAux[index[j]][3];
		differenceIndex = index[j + 1] - index[j];

		differenceR /= GLfloat(differenceIndex);
		differenceG /= GLfloat(differenceIndex);
		differenceB /= GLfloat(differenceIndex);
		differenceA /= GLfloat(differenceIndex);

		for (i = index[j] + 1; i < index[j + 1]; i++)
		{
			transFunctAux[i][0] = (transFunctAux[i - 1][0] + differenceR);
			transFunctAux[i][1] = (transFunctAux[i - 1][1] + differenceG);
			transFunctAux[i][2] = (transFunctAux[i - 1][2] + differenceB);
			transFunctAux[i][3] = (transFunctAux[i - 1][3] + differenceA);
		}
	}
	this->loadTransferFunct(transFunctAux);
}

void volumeRender::createBackFaceText()
{
	glBindTexture(GL_TEXTURE_2D, this->backFaceText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->screenWidth, this->screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool volumeRender::createFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->backFaceText, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void volumeRender::renderCube(glm::mat4 &MVP)
{
	this->backface.enable();
	glUniformMatrix4fv(this->backface.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	this->unitaryCube->display();
	this->backface.disable();
}
void volumeRender::renderCubeRayCast(glm::mat4 &MVP, glm::mat4 &model, glm::vec3 viewPos, glm::vec3 lightPos, bool on, glm::vec3 ambientComp, glm::vec3 diffuseComp, glm::vec3 specularComp)
{
	this->raycasting.enable();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, this->transferFunctionText);
	glUniform1i(this->raycasting.getLocation("transferFunctionText"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->backFaceText);
	glUniform1i(this->raycasting.getLocation("backFaceText"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, this->volumes[this->indexSelect]->volumeText);
	glUniform1i(this->raycasting.getLocation("volumeText"), 2);
	glUniform2f(this->raycasting.getLocation("screenSize"), (GLfloat)this->screenWidth, (GLfloat)this->screenHeight);
	glUniform1f(this->raycasting.getLocation("stepSize"), this->volumes[this->indexSelect]->step);
	glUniformMatrix4fv(this->raycasting.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(this->raycasting.getLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(this->raycasting.getLocation("viewPos"), 1, &viewPos[0]);
	glUniform3fv(this->raycasting.getLocation("lightPos"), 1, &lightPos[0]);
	glUniform3fv(this->raycasting.getLocation("ambientComp"), 1, &ambientComp[0]);
	glUniform3fv(this->raycasting.getLocation("diffuseComp"), 1, &diffuseComp[0]);
	glUniform3fv(this->raycasting.getLocation("specularComp"), 1, &specularComp[0]);
	glUniform1i(this->raycasting.getLocation("lighting"), on);
	this->unitaryCube->display();
	this->raycasting.disable();
}

void volumeRender::display(glm::mat4 &viewProjection, glm::vec3 viewPos, glm::vec3 lightPos, bool on, glm::vec3 ambientComp, glm::vec3 diffuseComp, glm::vec3 specularComp)
{
	if (this->indexSelect != -1)
	{
		glm::mat4 model, MVP;
		model = glm::translate(glm::mat4(1.0f), this->volumes[this->indexSelect]->translation) * glm::mat4_cast(this->volumes[this->indexSelect]->rotation) * glm::scale(model, glm::vec3(this->volumes[this->indexSelect]->escalation));
		MVP = viewProjection * model;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
		glViewport(0, 0, this->screenWidth, this->screenHeight);
		this->renderCube(MVP);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, this->screenWidth, this->screenHeight);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		this->renderCubeRayCast(MVP, model, viewPos, lightPos, on, ambientComp, diffuseComp, specularComp);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
}

void volumeRender::changeVolume(int type)
{
	if (this->indexSelect != -1)
	{
		if (type == 0)
			if (this->indexSelect > 0)
				this->indexSelect--;
			else
				this->indexSelect = this->volumes.size() - 1;
		else
			if (type == 1)
				if (this->indexSelect < this->volumes.size() - 1)
					this->indexSelect++;
				else
					this->indexSelect = 0;
	}
}

void volumeRender::resizeScreen(const glm::vec2 screen)
{
	this->screenWidth = (int)screen.x;
	this->screenHeight = (int)screen.y;
	this->createBackFaceText();
	this->createFrameBuffer();
}

void volumeRender::update_interface()
{
	if (visible_interface)
	{
		if (this->volumes[this->indexSelect]->translation != this->volume_interface->translation)
		{
			this->volumes[this->indexSelect]->translation = this->volume_interface->translation;
		}
		if (this->volumes[this->indexSelect]->rotation != this->volume_interface->rotation)
		{
			this->volumes[this->indexSelect]->rotation = this->volume_interface->rotation;
		}
		if (this->volumes[this->indexSelect]->escalation != this->volume_interface->scale)
		{
			this->volumes[this->indexSelect]->escalation = this->volume_interface->scale;
		}
	}
}