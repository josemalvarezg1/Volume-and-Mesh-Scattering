#include "TransferFunction.h"

double screenToWorldCoordX(double x, int gWidth)
{
	return((((x * 2.0) / double(gWidth)) - 1.0));
}

double screenToWorldCoordY(double y, int gHeight)
{
	return((1.0 - ((y * 2.0) / double(gHeight))));
}

double worldToRelativeCoord(double coord, double minCoord, double maxCoord)
{
	return (double((coord - minCoord) / (maxCoord - minCoord)));
}

glm::vec3 HSVtoRGB(double h, double s, double v)
{
	double c, x, m;
	glm::vec3 rgbAux(0.0);

	c = v * s;
	x = c * (1 - abs(fmod((h / 60.0), 2.0) - 1));
	m = v - c;

	if (0 <= h && h < 60)
		rgbAux = glm::vec3(c, x, 0.0);
	else
		if (60 <= h && h < 120)
			rgbAux = glm::vec3(x, c, 0.0);
		else
			if (120 <= h && h < 180)
				rgbAux = glm::vec3(0.0, c, x);
			else
				if (180 <= h && h < 240)
					rgbAux = glm::vec3(0.0, x, c);
				else
					if (240 <= h && h < 300)
						rgbAux = glm::vec3(x, 0.0, c);
					else
						if (300 <= h && h < 360)
							rgbAux = glm::vec3(c, 0.0, x);
	return rgbAux + glm::vec3(m);
}

quadColor::quadColor(std::vector<glm::vec2> coordsVertex, std::vector<glm::vec4> colors)
{
	this->coordsVertex = coordsVertex;
	this->colors = colors;
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	this->createVBO();
}

quadColor::~quadColor()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	this->coordsVertex.clear();
	this->colors.clear();
}

void quadColor::mobilize(double displX, double displY)
{
	for (unsigned int i = 0; i < this->coordsVertex.size(); i++)
		this->coordsVertex[i] = glm::vec2(this->coordsVertex[i].x + displX, this->coordsVertex[i].y + displY);
	createVBO();

}

void quadColor::createVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, ((sizeof(glm::vec2) + sizeof(glm::vec4)) * this->coordsVertex.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->coordsVertex.size(), &this->coordsVertex[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->coordsVertex.size(), sizeof(glm::vec4) * this->colors.size(), &this->colors[0]);
	glBindVertexArray(this->vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec2) * this->coordsVertex.size()));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void quadColor::display()
{
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->coordsVertex.size());
	glBindVertexArray(0);
}

quadTexture::quadTexture(double sizeX, double sizeY)
{
	glm::vec2 maxCoords, minCoords;
	maxCoords = glm::vec2((sizeX / 2.0), (sizeY / 2.0));
	minCoords = glm::vec2((-sizeX / 2.0), (-sizeY / 2.0f));
	this->coordsVertex.push_back(glm::vec2(minCoords.x, maxCoords.y));
	this->coordsVertex.push_back(glm::vec2(maxCoords.x, maxCoords.y));
	this->coordsVertex.push_back(glm::vec2(minCoords.x, minCoords.y));
	this->coordsVertex.push_back(glm::vec2(maxCoords.x, minCoords.y));
	this->coordsTexture.push_back(glm::vec2(0.0f, 0.0f));
	this->coordsTexture.push_back(glm::vec2(1.0f, 0.0f));
	this->coordsTexture.push_back(glm::vec2(0.0f, 1.0f));
	this->coordsTexture.push_back(glm::vec2(1.0f, 1.0f));
	this->createVBO();
}

quadTexture::~quadTexture()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	this->coordsVertex.clear();
	this->coordsTexture.clear();
}

void quadTexture::createVBO()
{
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * (this->coordsVertex.size() + this->coordsTexture.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * this->coordsVertex.size(), &this->coordsVertex[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * this->coordsVertex.size(), sizeof(glm::vec2) * this->coordsTexture.size(), &this->coordsTexture[0]);
	glBindVertexArray(this->vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec2) * this->coordsVertex.size()));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void quadTexture::display()
{
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->coordsVertex.size());
	glBindVertexArray(0);
}


region::region(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable)
{
	this->displacementX = dispX;
	this->displacementY = dispY;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->vertMovable = vertMovable;
	this->horizMovable = horizMovable;
}

glm::vec2 region::getDisp()
{
	return glm::vec2(displacementX, displacementY);
}

glm::vec2 region::getMinCoords()
{
	return glm::vec2(displacementX - (sizeX / 2.0), displacementY - (sizeY / 2.0));
}

glm::vec2 region::getMaxCoords()
{
	return glm::vec2(displacementX + (sizeX / 2.0), displacementY + (sizeY / 2.0));
}

void region::mobilize(double displX, double displY)
{
	this->displacementX = (this->displacementX + displX);
	this->displacementY = (this->displacementY + displY);
}

void region::editPos(double displX, double displY)
{
	if (this->horizMovable) this->displacementX = (this->displacementX + displX);
	if (this->vertMovable) this->displacementY = (this->displacementY + displY);
}

bool region::intersection(double displX, double displY)
{
	if (this->intersectionX(displX) && this->intersectionY(displY)) return true;
	return false;
}

bool region::intersectionX(double displX)
{
	if (((this->displacementX - (this->sizeX / 2.0)) < displX) && (displX < (this->displacementX + (this->sizeX / 2.0)))) return true;
	return false;
}

bool region::intersectionY(double displY)
{
	if (((this->displacementY - (this->sizeY / 2.0)) < displY) && (displY < (this->displacementY + (this->sizeY / 2.0)))) return true;
	return false;
}

point::point(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable, double coordH, glm::vec2 coordSV) : region(dispX, dispY, sizeX, sizeY, horizMovable, vertMovable)
{
	this->coordH = coordH;
	this->coordSV = coordSV;
}

void point::mobilizeSP(double displX, double displY)
{
	this->coordH = (this->coordH + displX);
	this->coordSV = this->coordSV + glm::vec2(displX, displY);
}

histogram::histogram(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable) : region(dispX, dispY, sizeX, sizeY, horizMovable, vertMovable)
{
	point *startPoint, *finalPoint;
	std::vector<glm::vec2> coordsVertex;
	std::vector<glm::vec4> colors;
	this->cartesianMap = new region(MAPDISPX, MAPDISPY, MAPSIZEX, MAPSIZEY, true, true);
	startPoint = new point((dispX - (this->cartesianMap->sizeX / 2.0) + ERROR), (dispY - (this->cartesianMap->sizeY / 2.0) + ERROR), POINTSIZEX, POINTSIZEY, false, true, SELECTORDISPX, glm::vec2(POINTDISPX, AREADISPY - (AREASIZEY / 2.0)));
	finalPoint = new point((dispX + (this->cartesianMap->sizeX / 2.0) - ERROR), (dispY + (this->cartesianMap->sizeY / 2.0) - ERROR), POINTSIZEX, POINTSIZEY, false, true, SELECTORDISPX, glm::vec2(POINTDISPX, POINTDISPY));
	this->controlPoints.push_back(startPoint);
	this->controlPoints.push_back(finalPoint);
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 0.0));
	coordsVertex.push_back(glm::vec2(this->controlPoints[0]->displacementX, this->controlPoints[0]->displacementY));
	coordsVertex.push_back(glm::vec2(this->controlPoints[0]->displacementX, this->controlPoints[0]->displacementY));
	coordsVertex.push_back(glm::vec2(this->controlPoints[1]->displacementX, this->controlPoints[1]->displacementY));
	coordsVertex.push_back(glm::vec2(this->controlPoints[1]->displacementX, this->controlPoints[0]->displacementY));
	this->connections = new quadColor(coordsVertex, colors);
}

void histogram::mobilizeH(double displX, double displY)
{
	this->mobilize(displX, displY);
	this->cartesianMap->mobilize(displX, displY);
	for (unsigned int i = 0; i < this->controlPoints.size(); i++)
	{
		this->controlPoints[i]->mobilize(displX, displY);
		this->controlPoints[i]->mobilizeSP(displX, displY);
	}
	this->connections->mobilize(displX, displY);
}

int histogram::selected(double dispX, double dispY)
{
	for (unsigned int i = 0; i < this->controlPoints.size(); ++i) if (this->controlPoints[i]->intersection(dispX, dispY)) return i;
	return -1;
}

bool histogram::collision(double dispX, double dispY, int indexSelect)
{
	glm::vec2 disp1, disp2;
	double lowerLimitX, upperLimitX;
	disp1 = this->controlPoints[indexSelect]->getDisp() + glm::vec2(dispX, dispY);
	lowerLimitX = this->controlPoints[indexSelect]->displacementX;
	upperLimitX = disp1.x;
	for (unsigned int i = 0; i < this->controlPoints.size(); ++i)
	{
		if (i != indexSelect)
		{
			disp2 = this->controlPoints[i]->getDisp();
			if ((this->circlesIntersection(disp1, disp2)) || (this->between(lowerLimitX + (POINTSIZEX / 2.0), disp2.x, upperLimitX + (POINTSIZEX / 2.0))) || (this->between(upperLimitX - (POINTSIZEX / 2.0), disp2.x, lowerLimitX - (POINTSIZEX / 2.0))))
				return true;
		}
	}
	return false;
}

bool histogram::circlesIntersection(glm::vec2 circle1, glm::vec2 circle2)
{
	if ((pow((circle2.x - circle1.x), 2) + pow((circle1.y - circle2.y), 2)) <= (pow((POINTSIZEX), 2))) return true;
	return false;
}

bool histogram::between(double lowerLimit, double position, double upperLimit)
{
	if ((lowerLimit < position) && (upperLimit > position)) return true;
	return false;
}

bool histogram::creation(double dispX)
{
	for (unsigned int i = 0; i < this->controlPoints.size(); i++) if (this->controlPoints[i]->intersectionX(dispX)) return true;
	return false;
}

void histogram::updateConnections(glm::vec2 coordsXS, glm::vec2 coordsPX, glm::vec2 coordsPY, glm::vec2 coordsYPS)
{
	double h, s, v, alpha;
	glm::vec3 color;
	this->connections->coordsVertex.clear();
	this->connections->colors.clear();
	for (unsigned int i = 0; i < this->controlPoints.size(); i++)
	{
		h = worldToRelativeCoord(this->controlPoints[i]->coordH, coordsXS[0], coordsXS[1]) * 360.0;
		s = worldToRelativeCoord(this->controlPoints[i]->coordSV.x, coordsPX[0], coordsPX[1]);
		v = worldToRelativeCoord(this->controlPoints[i]->coordSV.y, coordsPY[0], coordsPY[1]);
		alpha = worldToRelativeCoord(this->controlPoints[i]->displacementY, coordsYPS[0], coordsYPS[1]);
		color = HSVtoRGB(h, s, v);
		this->connections->colors.push_back(glm::vec4(color, alpha));
		this->connections->colors.push_back(glm::vec4(color, 0.0));
		this->connections->coordsVertex.push_back(glm::vec2(this->controlPoints[i]->displacementX, this->controlPoints[i]->displacementY));
		this->connections->coordsVertex.push_back(glm::vec2(this->controlPoints[i]->displacementX, this->displacementY - (this->cartesianMap->sizeY / 2.0)));
	}
	this->connections->createVBO();
}

colorPick::colorPick()
{
	std::vector<glm::vec2> coordsVertex;
	std::vector<glm::vec4> colors;
	double i;
	for (i = 0; i <= (BARSIZEX * 6); i += BARSIZEX)
	{
		coordsVertex.push_back(glm::vec2(BARDISPX + i, BARDISPY + (BARSIZEY / 2.0)));
		coordsVertex.push_back(glm::vec2(BARDISPX + i, BARDISPY - (BARSIZEY / 2.0)));
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
	this->colorBarQ = new quadColor(coordsVertex, colors);
	coordsVertex.clear();
	colors.clear();
	for (i = 0; i <= AREASIZEX; i += AREASIZEX)
	{
		coordsVertex.push_back(glm::vec2(AREADISPX + i, AREADISPY + (AREASIZEY / 2.0)));
		coordsVertex.push_back(glm::vec2(AREADISPX + i, AREADISPY - (AREASIZEY / 2.0)));
	}
	colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(1.0, 0.0, 0.0, 1.0));
	colors.push_back(glm::vec4(0.0, 0.0, 0.0, 1.0));
	this->colorAreaQ = new quadColor(coordsVertex, colors);
	coordsVertex.clear();
	colors.clear();
	for (i = 0; i <= COLORSSIZEX; i += COLORSSIZEX)
	{
		coordsVertex.push_back(glm::vec2(COLORSDISPX + i, COLORSDISPY + (COLORSSIZEY / 2.0)));
		coordsVertex.push_back(glm::vec2(COLORSDISPX + i, COLORSDISPY - (COLORSSIZEY / 2.0)));
		colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
		colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0));
	}
	this->colorSelectQ = new quadColor(coordsVertex, colors);
	this->selector = new region(SELECTORDISPX, SELECTORDISPY, SELECTORSIZEX, SELECTORSIZEY, true, false);
	this->point = new region(POINTDISPX, POINTDISPY, POINTSIZEX, POINTSIZEY, true, true);
	this->colorBarR = new region((BARDISPX + (BARSIZEX * 3)), BARDISPY, (BARSIZEX * 6), BARSIZEY, false, false);
	this->colorAreaR = new region((COLORSDISPX + AREASIZEX / 2.0), AREADISPY, AREASIZEX, AREASIZEY, false, false);
}

colorPick::~colorPick()
{
	this->colorBarQ->~quadColor();
	this->colorAreaQ->~quadColor();
	this->colorSelectQ->~quadColor();
}

void colorPick::mobilize(double displX, double displY)
{
	this->colorBarQ->mobilize(displX, displY);
	this->colorAreaQ->mobilize(displX, displY);
	this->colorSelectQ->mobilize(displX, displY);
	this->selector->mobilize(displX, displY);
	this->point->mobilize(displX, displY);
	this->colorBarR->mobilize(displX, displY);
	this->colorAreaR->mobilize(displX, displY);
}


interfaceFunction::interfaceFunction()
{
	quadTexture *pointQuad, *selectorQuad, *histogramQuad;
	texture *pointText, *selectorText, *histogramText, *pointSelectText;
	pointText = new texture("../images/point.png");
	selectorText = new texture("../images/selector.png");
	histogramText = new texture("../images/histogram.png");
	pointSelectText = new texture("../images/pointSelect.png");
	pointQuad = new quadTexture(POINTSIZEX, POINTSIZEY);
	selectorQuad = new quadTexture(SELECTORSIZEX, SELECTORSIZEY);
	histogramQuad = new quadTexture(HISTSIZEX, HISTSIZEY);
	this->map = new histogram(HISTDISPX, HISTDISPY, HISTSIZEX, HISTSIZEY, false, false);
	this->colorPicker = new colorPick();
	this->quadsT.push_back(pointQuad);
	this->quadsT.push_back(selectorQuad);
	this->quadsT.push_back(histogramQuad);
	this->textures.push_back(pointText);
	this->textures.push_back(selectorText);
	this->textures.push_back(histogramText);
	this->textures.push_back(pointSelectText);
	this->histogramPress = false;
	this->cartesianMapPress = false;
	this->colorBarPress = false;
	this->colorAreaPress = false;
	this->hide = false;
	this->movable = false;
	this->indexSelect = -1;
	this->initShaders();
}

interfaceFunction::~interfaceFunction()
{
	this->quadsT.clear();
	this->textures.clear();
}

void interfaceFunction::initShaders()
{
	this->p1.loadShader("../src/shaders/quad.vert", CGLSLProgram::VERTEX);
	this->p1.loadShader("../src/shaders/quad.frag", CGLSLProgram::FRAGMENT);
	this->p2.loadShader("../src/shaders/function.vert", CGLSLProgram::VERTEX);
	this->p2.loadShader("../src/shaders/function.frag", CGLSLProgram::FRAGMENT);

	this->p1.create_link();
	this->p2.create_link();

	this->p1.enable();
	this->p1.addAttribute("vertCoords");
	this->p1.addAttribute("texCoords");
	this->p1.addUniform("textureId");
	this->p1.addUniform("displacement");
	this->p1.disable();

	this->p2.enable();
	this->p2.addAttribute("vertCoords");
	this->p2.addAttribute("color");
	this->p2.disable();
}

void interfaceFunction::renderHistogram()
{
	glBindTexture(GL_TEXTURE_2D, this->textures[HISTOGRAM]->get_texture_ID());
	this->p1.enable();
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->map->displacementX, (GLfloat) this->map->displacementY);
	glUniform1i(this->p1.getLocation("textureId"), 0);
	this->quadsT[HISTOGRAM]->display();
	this->p1.disable();
}

void interfaceFunction::renderFunction()
{
	this->p2.enable();
	this->map->connections->display();
	this->p2.disable();
}

void interfaceFunction::renderPoints()
{
	this->p1.enable();
	for (unsigned int i = 0; i < this->map->controlPoints.size(); i++)
	{
		if (this->indexSelect == i)
			glBindTexture(GL_TEXTURE_2D, this->textures[POINTSELECT]->get_texture_ID());
		else
			glBindTexture(GL_TEXTURE_2D, this->textures[POINTC]->get_texture_ID());
		glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->map->controlPoints[i]->displacementX, (GLfloat) this->map->controlPoints[i]->displacementY);
		this->quadsT[POINTC]->display();
	}
	this->p1.disable();
}

void interfaceFunction::renderColorPicker()
{
	this->p2.enable();
	this->colorPicker->colorBarQ->display();
	this->colorPicker->colorAreaQ->display();
	this->colorPicker->colorSelectQ->display();
	this->p2.disable();
}

void interfaceFunction::renderSelectorAndPoint()
{
	this->p1.enable();
	glBindTexture(GL_TEXTURE_2D, this->textures[SELECTOR]->get_texture_ID());
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->colorPicker->selector->displacementX, (GLfloat) this->colorPicker->selector->displacementY);
	glUniform1i(this->p1.getLocation("textureId"), 0);
	this->quadsT[SELECTOR]->display();
	glBindTexture(GL_TEXTURE_2D, this->textures[POINTC]->get_texture_ID());
	glUniform2f(this->p1.getLocation("displacement"), (GLfloat) this->colorPicker->point->displacementX, (GLfloat) this->colorPicker->point->displacementY);
	this->quadsT[POINTC]->display();
	this->p1.disable();
}

void interfaceFunction::display()
{
	if (!hide)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		renderHistogram();
		renderFunction();
		renderPoints();
		renderColorPicker();
		renderSelectorAndPoint();

		glDisable(GL_BLEND);
	}
}

void interfaceFunction::mobilize(double displX, double displY)
{
	this->map->mobilizeH(displX, displY);
	this->colorPicker->mobilize(displX, displY);
}

void interfaceFunction::updateCoords()
{
	if (this->indexSelect != -1 && !this->colorBarPress && !this->colorAreaPress)
	{
		if (this->colorPicker->point->displacementX != this->map->controlPoints[this->indexSelect]->coordSV.x || this->colorPicker->point->displacementY != this->map->controlPoints[this->indexSelect]->coordSV.y || this->colorPicker->selector->displacementX != this->map->controlPoints[this->indexSelect]->coordH)
		{
			this->colorPicker->selector->displacementX = this->map->controlPoints[this->indexSelect]->coordH;
			this->colorPicker->point->displacementX = this->map->controlPoints[this->indexSelect]->coordSV.x;
			this->colorPicker->point->displacementY = this->map->controlPoints[this->indexSelect]->coordSV.y;
			this->updateColorArea();
			this->updateColorSelect();
		}
	}
}

void interfaceFunction::updateLines()
{
	glm::vec2 coordsXS, coordsXP, coordsYP, coordsYPS;
	coordsXS = glm::vec2(this->colorPicker->colorBarR->displacementX - (this->colorPicker->colorBarR->sizeX / 2.0), this->colorPicker->colorBarR->displacementX + (this->colorPicker->colorBarR->sizeX / 2.0));
	coordsXP = glm::vec2(this->colorPicker->colorAreaR->displacementX - (this->colorPicker->colorAreaR->sizeX / 2.0), this->colorPicker->colorAreaR->displacementX + (this->colorPicker->colorAreaR->sizeX / 2.0));
	coordsYP = glm::vec2(this->colorPicker->colorAreaR->displacementY - (this->colorPicker->colorAreaR->sizeY / 2.0), this->colorPicker->colorAreaR->displacementY + (this->colorPicker->colorAreaR->sizeY / 2.0));
	coordsYPS = glm::vec2(this->map->cartesianMap->displacementY - (this->map->cartesianMap->sizeY / 2.0), this->map->cartesianMap->displacementY + (this->map->cartesianMap->sizeY / 2.0));
	this->map->updateConnections(coordsXS, coordsXP, coordsYP, coordsYPS);
}

void interfaceFunction::updateColorArea()
{
	glm::vec3 color;
	double h;
	h = worldToRelativeCoord(this->colorPicker->selector->displacementX, this->colorPicker->colorBarR->displacementX - (this->colorPicker->colorBarR->sizeX / 2.0), this->colorPicker->colorBarR->displacementX + (this->colorPicker->colorBarR->sizeX / 2.0)) * 360.0;
	color = HSVtoRGB(h, 1.0, 1.0);
	this->colorPicker->colorAreaQ->colors[2] = glm::vec4(color, 1.0);
	this->colorPicker->colorAreaQ->createVBO();
}

void interfaceFunction::updateColorSelect()
{
	double h, s, v;
	glm::vec3 color;
	h = worldToRelativeCoord(this->colorPicker->selector->displacementX, this->colorPicker->colorBarR->displacementX - (this->colorPicker->colorBarR->sizeX / 2.0), this->colorPicker->colorBarR->displacementX + (this->colorPicker->colorBarR->sizeX / 2.0)) * 360.0;
	s = worldToRelativeCoord(this->colorPicker->point->displacementX, this->colorPicker->colorAreaR->displacementX - (this->colorPicker->colorAreaR->sizeX / 2.0), this->colorPicker->colorAreaR->displacementX + (this->colorPicker->colorAreaR->sizeX / 2.0));
	v = worldToRelativeCoord(this->colorPicker->point->displacementY, this->colorPicker->colorAreaR->displacementY - (this->colorPicker->colorAreaR->sizeX / 2.0), this->colorPicker->colorAreaR->displacementY + (this->colorPicker->colorAreaR->sizeX / 2.0));
	color = HSVtoRGB(h, s, v);
	for (unsigned int i = 0; i < this->colorPicker->colorSelectQ->colors.size(); i++) this->colorPicker->colorSelectQ->colors[i] = glm::vec4(color, 1.0);
	this->colorPicker->colorSelectQ->createVBO();
}

bool interfaceFunction::clickTransferF(double x, double y, int gWidth, int gHeight)
{
	if (!hide)
	{
		double coordX, coordY;
		if (!histogramPress && !cartesianMapPress && !colorBarPress && !colorAreaPress)
		{
			coordX = screenToWorldCoordX(x, gWidth);
			coordY = screenToWorldCoordY(y, gHeight);

			if (this->map->intersection(coordX, coordY))
			{
				if (this->map->cartesianMap->intersection(coordX, coordY))
				{
					this->indexSelect = this->map->selected(coordX, coordY);
					if (this->indexSelect == -1 && !this->map->creation(coordX))
					{
						point *pointNew;
						pointNew = new point(coordX, coordY, POINTSIZEX, POINTSIZEY, true, true, this->colorPicker->selector->displacementX, this->colorPicker->point->getDisp());
						this->map->controlPoints.push_back(pointNew);
						std::sort(this->map->controlPoints.begin(), this->map->controlPoints.end(), [](const point *lhs, const point *rhs) { return lhs->displacementX < rhs->displacementX; });
						indexSelect = this->map->selected(coordX, coordY);
						updateLines();
					}
					if (this->indexSelect != -1)
					{
						this->xReference = coordX;
						this->yReference = coordY;
						this->updateColorArea();
						this->updateColorSelect();
						this->cartesianMapPress = true;
					}
				}
				else
				{
					this->xReference = coordX;
					this->yReference = coordY;
					this->indexSelect = -1;
					histogramPress = true;
				}
				return true;
			}
			else
			{
				if (this->colorPicker->colorBarR->intersection(coordX, coordY))
				{
					if (!this->colorPicker->selector->intersection(coordX, coordY))
					{
						this->colorPicker->selector->displacementX = coordX;
						if (this->indexSelect != -1)
						{
							this->map->controlPoints[indexSelect]->coordH = this->colorPicker->selector->displacementX;
							this->updateLines();
						}
						this->updateColorArea();
						this->updateColorSelect();
					}
					this->xReference = coordX;
					this->yReference = coordY;
					this->colorBarPress = true;
					return true;
				}
				else
				{
					if (this->colorPicker->colorAreaR->intersection(coordX, coordY))
					{
						if (!this->colorPicker->point->intersection(coordX, coordY))
						{
							this->colorPicker->point->displacementX = coordX;
							this->colorPicker->point->displacementY = coordY;
							if (this->indexSelect != -1)
							{
								this->map->controlPoints[indexSelect]->coordSV.x = (float) this->colorPicker->point->displacementX;
								this->map->controlPoints[indexSelect]->coordSV.y = (float) this->colorPicker->point->displacementY;
								this->updateLines();
							}
							this->updateColorSelect();
						}
						this->xReference = coordX;
						this->yReference = coordY;
						this->colorAreaPress = true;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void interfaceFunction::disableSelect()
{
	this->histogramPress = false;
	this->cartesianMapPress = false;
	this->colorBarPress = false;
	this->colorAreaPress = false;
}
bool interfaceFunction::poscursorTransferF(double x, double y, int gWidth, int gHeight)
{
	if (!hide)
	{
		double dispX, dispY;

		dispX = screenToWorldCoordX(x, gWidth);
		dispY = screenToWorldCoordY(y, gHeight);

		if (this->histogramPress)
		{
			if (this->movable)
				this->mobilize((dispX - this->xReference), (dispY - this->yReference));
			this->xReference = dispX;
			this->yReference = dispY;
			return true;
		}

		if (this->cartesianMapPress)
		{
			if (!this->map->collision((dispX - this->xReference), (dispY - this->yReference), this->indexSelect) && this->map->cartesianMap->intersection(this->map->controlPoints[indexSelect]->displacementX + (dispX - this->xReference), this->map->controlPoints[indexSelect]->displacementY + (dispY - this->yReference)))
			{
				this->map->controlPoints[indexSelect]->editPos((dispX - this->xReference), (dispY - this->yReference));
				this->map->controlPoints[indexSelect]->coordH = this->colorPicker->selector->displacementX;
				this->map->controlPoints[indexSelect]->coordSV = this->colorPicker->point->getDisp();
				this->updateLines();
			}
			this->xReference = dispX;
			this->yReference = dispY;
			return true;
		}

		if (this->colorBarPress)
		{
			if (this->colorPicker->colorBarR->intersectionX(this->colorPicker->selector->displacementX + (dispX - this->xReference)))
			{
				this->colorPicker->selector->editPos((dispX - this->xReference), (dispY - this->yReference));
				if (this->indexSelect != -1)
				{
					this->map->controlPoints[indexSelect]->coordH = this->colorPicker->selector->displacementX;
					this->updateLines();
				}
				this->updateColorArea();
				this->updateColorSelect();
			}
			this->xReference = dispX;
			this->yReference = dispY;
			return true;
		}

		if (this->colorAreaPress)
		{
			if (this->colorPicker->colorAreaR->intersection(this->colorPicker->point->displacementX + (dispX - this->xReference), this->colorPicker->point->displacementY + (dispY - this->yReference)))
			{
				this->colorPicker->point->editPos((dispX - this->xReference), (dispY - this->yReference));
				if (this->indexSelect != -1)
				{
					this->map->controlPoints[indexSelect]->coordSV.x = (float) this->colorPicker->point->displacementX;
					this->map->controlPoints[indexSelect]->coordSV.y = (float) this->colorPicker->point->displacementY;
					this->updateLines();
				}
				this->updateColorSelect();
			}
			this->xReference = dispX;
			this->yReference = dispY;
			return true;
		}
	}
	return false;
}

void interfaceFunction::deletePoint()
{
	if (this->indexSelect > 0 && this->indexSelect != this->map->controlPoints.size() - 1)
	{
		this->map->controlPoints.erase(this->map->controlPoints.begin() + (this->indexSelect));
		this->indexSelect = -1;
		this->updateLines();
	}
}

std::vector<double> interfaceFunction::getColorPoints()
{
	std::vector<double> points;
	double valueX, h, s, v, alpha;
	glm::vec3 rgb;

	for (unsigned int i = 0; i < this->map->controlPoints.size(); ++i)
	{
		valueX = worldToRelativeCoord(this->map->controlPoints[i]->displacementX, this->map->cartesianMap->displacementX - (this->map->cartesianMap->sizeX / 2.0), this->map->cartesianMap->displacementX + (this->map->cartesianMap->sizeX / 2.0));
		h = worldToRelativeCoord(this->map->controlPoints[i]->coordH, this->colorPicker->colorBarR->displacementX - (this->colorPicker->colorBarR->sizeX / 2.0), this->colorPicker->colorBarR->displacementX + (this->colorPicker->colorBarR->sizeX / 2.0)) * 360.0;
		s = worldToRelativeCoord(this->map->controlPoints[i]->coordSV.x, this->colorPicker->colorAreaR->displacementX - (this->colorPicker->colorAreaR->sizeX / 2.0), this->colorPicker->colorAreaR->displacementX + (this->colorPicker->colorAreaR->sizeX / 2.0));
		v = worldToRelativeCoord(this->map->controlPoints[i]->coordSV.y, this->colorPicker->colorAreaR->displacementY - (this->colorPicker->colorAreaR->sizeY / 2.0), this->colorPicker->colorAreaR->displacementY + (this->colorPicker->colorAreaR->sizeY / 2.0));
		alpha = worldToRelativeCoord(this->map->controlPoints[i]->displacementY, this->map->cartesianMap->displacementY - (this->map->cartesianMap->sizeY / 2.0), this->map->cartesianMap->displacementY + (this->map->cartesianMap->sizeY / 2.0));
		rgb = HSVtoRGB(h, s, v);
		points.push_back(valueX);
		points.push_back(rgb.r);
		points.push_back(rgb.g);
		points.push_back(rgb.b);
		points.push_back(alpha);
	}
	return points;
}