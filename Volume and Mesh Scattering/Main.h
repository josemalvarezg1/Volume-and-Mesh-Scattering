#pragma once
#define GLFW_DLL
#define GLEW_STATIC

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
#include "Model.h"
#include "Halton.h"
#include "ScatteredMap.h"
#include "TransferFunction.h"
#include "Volume.h"

void reshape(GLFWwindow *window, int width, int height);
void key_input(GLFWwindow *window, int key, int scan_code, int action, int mods);
void click(GLFWwindow* window, int button, int action, int mods);
void scroll(GLFWwindow* window, double x_offset, double y_offset);
void pos_cursor(GLFWwindow* window, double x, double y);
void char_input(GLFWwindow* window, unsigned int scan_char);
void drop_path(GLFWwindow* window, int count, const char** paths);
void movement();
bool init_glfw();
bool init_glew();
bool init_ant_tweak_bar();
bool init_scene();
void display();
void destroy();
int main();