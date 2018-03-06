#pragma once

#define GLFW_DLL
#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <string>
//#include "../headers/transferFunction.hpp"
//#include "../headers/volume.hpp"
#include "Camera.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AntTweakBar.h>

void rescale(GLFWwindow *window, int width, int height);
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void click(GLFWwindow* window, int button, int action, int mods);
void posCursor(GLFWwindow* window, double x, double y);
void scroll(GLFWwindow* window, double xoffset, double yoffset);
void charInput(GLFWwindow* window, unsigned int scanChar);
void dropPath(GLFWwindow* window, int count, const char** paths);
bool initGlfw();
bool initGlew();
void initScene();
void display();
void destroy();
int main();