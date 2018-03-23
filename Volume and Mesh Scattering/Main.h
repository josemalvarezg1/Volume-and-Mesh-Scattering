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

glm::vec3 scattering_coefficients[] = { { glm::vec3(0.68f, 0.70f, 0.55f) },			// Patata
											  { glm::vec3(2.19f, 2.62f, 3.00f) },	// Mármol
											  { glm::vec3(0.74f, 0.88f, 1.01f) },	// Piel
											  { glm::vec3(2.55f, 3.21f, 3.77f) },	// Leche
											  { glm::vec3(7.38f, 5.47f, 3.1f) },	// Crema
											  { glm::vec3(1.0f, 1.0f, 1.0f) } };	// Ninguno

glm::vec3 absorption_coefficients[] = { { glm::vec3(0.0024f, 0.0090f, 0.12f) },		// Patata
										{ glm::vec3(0.0021f, 0.0041f, 0.0071f) },	// Mármol
										{ glm::vec3(0.032f, 0.17f, 0.48f) },		// Piel
										{ glm::vec3(0.0011f, 0.0024f, 0.014f) },	// Leche
										{ glm::vec3(0.0002f, 0.0028f, 0.016f) },	// Crema
										{ glm::vec3(0.0f, 0.0f, 0.0f) } };			// Ninguno

glm::vec3 diffuse_reflectances[] = { { glm::vec3(0.77f, 0.62f, 0.21f) },	// Patata
									 { glm::vec3(0.83f, 0.79f, 0.75f) },	// Mármol
									 { glm::vec3(0.44f, 0.22f, 0.13f) },	// Piel
									 { glm::vec3(0.91f, 0.88f, 0.76f) },	// Leche
									 { glm::vec3(0.98f, 0.90f, 0.7f) },		// Crema
									 { glm::vec3(1.0f, 1.0f, 1.0f) } };		// Ninguno

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
float calculate_c_phi(float ni);
float calculate_c_e(float ni);
bool initGlfw();
bool initGlew();
bool initAntTweakBar();
bool initScene();
void display();
void destroy();
int main();