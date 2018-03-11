#pragma once

#define GLFW_DLL
#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define BUFFER_OFFSET(offset) ((char*)NULL + (offset))

#include <iostream>
#include <string.h>
//#include "../headers/transferFunction.hpp"
//#include "../headers/volume.hpp"
#include "Camera.h"
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AntTweakBar.h>
#include "GLSLProgram.h"
#include "Light.h"
using namespace std;

void reshape(GLFWwindow *window, int width, int height);
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void click(GLFWwindow* window, int button, int action, int mods);
void posCursor(GLFWwindow* window, double x, double y);
void scroll(GLFWwindow* window, double xoffset, double yoffset);
void charInput(GLFWwindow* window, unsigned int scanChar);
void dropPath(GLFWwindow* window, int count, const char** paths);
bool initGlfw();
bool initGlew();
bool initScene();
void display();
void destroy();
int main();