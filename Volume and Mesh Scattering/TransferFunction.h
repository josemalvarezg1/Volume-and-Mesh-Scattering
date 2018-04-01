#pragma once
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include <iostream>
#include <vector>
#include <algorithm>
#include "GLSLProgram.h"
#include "Texture.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

enum structure
{
	POINTC,
	SELECTOR,
	HISTOGRAM,
	POINTSELECT,
};

#define ERROR 0.0001
#define HISTDISPX 0.65
#define HISTDISPY 0.65
#define HISTSIZEX 0.6
#define HISTSIZEY 0.6
#define MAPDISPX 0.65
#define MAPDISPY 0.65
#define MAPSIZEX 0.472
#define MAPSIZEY 0.3712
#define POINTDISPX 0.351
#define POINTDISPY 0.23
#define POINTSIZEX 0.015
#define POINTSIZEY 0.015
#define SELECTORDISPX 0.351
#define SELECTORDISPY 0.291
#define SELECTORSIZEX 0.01
#define SELECTORSIZEY 0.1
#define BARDISPX 0.35
#define BARDISPY 0.29
#define BARSIZEX 0.1
#define BARSIZEY 0.1
#define AREADISPX 0.35
#define AREADISPY -0.12
#define AREASIZEX 0.6
#define AREASIZEY 0.7
#define COLORSDISPX 0.35
#define COLORSDISPY -0.53
#define COLORSSIZEX 0.6
#define COLORSSIZEY 0.1

class quadColor
{
public:
	std::vector<glm::vec2> coordsVertex;
	std::vector<glm::vec4> colors;
	GLuint vbo, vao;

	quadColor(std::vector<glm::vec2> coordsVertex, std::vector<glm::vec4> colors);
	~quadColor();
	void mobilize(double displX, double displY);
	void createVBO();
	void display();
};

class quadTexture
{
public:
	std::vector<glm::vec2> coordsVertex, coordsTexture;
	GLuint vbo, vao;

	quadTexture(double sizeX, double sizeY);
	~quadTexture();
	void createVBO();
	void display();
};

class region
{
public:
	double displacementX, displacementY, sizeX, sizeY;
	bool vertMovable, horizMovable;

	region(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable);
	glm::vec2 getDisp();
	glm::vec2 getMinCoords();
	glm::vec2 getMaxCoords();
	void mobilize(double displX, double displY);
	void editPos(double displX, double displY);
	bool intersection(double displX, double displY);
	bool intersectionX(double displX);
	bool intersectionY(double displY);
};

class point : public region
{
public:
	double coordH;
	glm::vec2 coordSV;

	point(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable, double coordH, glm::vec2 coordSV);
	void mobilizeSP(double displX, double displY);
};

class histogram : public region
{
public:
	std::vector<point*> controlPoints;
	region *cartesianMap;
	quadColor *connections;

	histogram(double dispX, double dispY, double sizeX, double sizeY, bool horizMovable, bool vertMovable);
	void mobilizeH(double displX, double displY);
	int selected(double dispX, double dispY);
	bool collision(double dispX, double dispY, int indexSelect);
	bool circlesIntersection(glm::vec2 circle1, glm::vec2 circle2);
	bool between(double lowerLimit, double position, double upperLimit);
	bool creation(double dispX);
	void updateConnections(glm::vec2 coordsXS, glm::vec2 coordsPX, glm::vec2 coordsPY, glm::vec2 coordsYPS);
};

class colorPick
{
public:
	quadColor *colorBarQ, *colorAreaQ, *colorSelectQ;
	region *selector, *point, *colorBarR, *colorAreaR;

	colorPick();
	~colorPick();
	void mobilize(double displX, double displY);
};

class interfaceFunction
{
public:
	histogram *map;
	colorPick *colorPicker;
	std::vector<quadTexture*> quadsT;
	std::vector<texture*> textures;
	bool histogramPress, cartesianMapPress, colorBarPress, colorAreaPress, hide, movable;
	int indexSelect;
	double xReference, yReference;
	CGLSLProgram p1, p2;

	interfaceFunction();
	~interfaceFunction();
	void initShaders();
	void renderHistogram();
	void renderFunction();
	void renderPoints();
	void renderColorPicker();
	void renderSelectorAndPoint();
	void display();
	void mobilize(double displX, double displY);
	void updateCoords();
	void updateLines();
	void updateColorArea();
	void updateColorSelect();
	bool clickTransferF(double x, double y, int gWidth, int gHeight);
	void disableSelect();
	bool poscursorTransferF(double x, double y, int gWidth, int gHeight);
	void deletePoint();
	std::vector<double> getColorPoints();
};