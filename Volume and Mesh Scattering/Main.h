#pragma once
#define GLFW_DLL
#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <string.h>
#include "Camera.h"
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AntTweakBar.h>
#include <random>
#include "GLSLProgram.h"
#include "Light.h"
#include "G-Buffer.h"
#include "Material.h"

void reshape(GLFWwindow *window, int width, int height);
void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void click(GLFWwindow* window, int button, int action, int mods);
void scroll(GLFWwindow* window, double xoffset, double yoffset);
void posCursor(GLFWwindow* window, double x, double y);
void charInput(GLFWwindow* window, unsigned int scanChar);
void dropPath(GLFWwindow* window, int count, const char** paths);
void movement();
int negative_positive();
void generate_ortographic_cameras();
float lerp(float a, float b, float f);
void generateSamples();
bool initGlfw();
bool initGlew();
bool initAntTweakBar();
bool initScene();
void display();
void destroy();
int main();