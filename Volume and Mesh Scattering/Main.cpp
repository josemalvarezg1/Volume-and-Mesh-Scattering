#include "Main.h"
#include "Model.h"
#include "Halton.h"
#include "ScatteredMap.h"

GLFWwindow *gWindow;
int g_width, g_height;
GLuint num_of_lights, num_of_orto_cameras, num_of_samples_per_frag;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f;
GLdouble lastX = 600.0, lastY = 340.0;
bool keys[1024], keysPressed[1024], selectingModel = false, selectingLight = false, firstMouse = true, activateCamera = false;
std::vector<ScatteredMap*> cameraPositions;
std::vector<glm::vec3> samples;
glm::mat4 projection, view, model;

light *scene_light;
camera *scene_camera;
meshSet *mSet;
light_buffers_set *light_buffers;

CGLSLProgram glslProgram, glslGBuffer, glslGBufferP, glslScatteredMap;
int selectedModel = -1;

GLuint quadVAO, quadVBO;

// Variables pre-calculadas
glm::vec3 attenuation_coeff, albedo, scattering_coeff_prime, attenuation_coeff_prime, albedo_prime;
glm::vec3 D, effective_transport_coeff, de, zr;
float c_phi_1, c_phi_2, c_e, A;

void render_quad()
{
	if (quadVAO == 0) {
		GLfloat quadVertices[] = {
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void reshape(GLFWwindow *window, int width, int height)
{
	g_width = max(width, 1);
	g_height = max(height, 1);

	scene_light->light_interface->reshape(g_width, g_height);
	mSet->model_interface->reshape(g_width, g_height);

	for (size_t i = 0; i < num_of_lights; i++)
		light_buffers->array_of_buffers[0]->update_g_buffer(g_width, g_height);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glViewport(0, 0, g_width, g_height);
}

void keyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (TwEventKeyGLFW(key, action))
		return;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			keysPressed[key] = false;
		}

		if (key == GLFW_KEY_T && (action == GLFW_PRESS)) {
			activateCamera = !activateCamera;

			if (activateCamera) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				if (selectingModel) {
					mSet->not_click_model();
				}
				if (selectingLight) {
					scene_light->not_click_light();
				}
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (selectingModel) {
					mSet->click_model(selectedModel);
				}
				if (selectingLight) {
					scene_light->click_light();
				}
			}
		}
	}
}

void click(GLFWwindow* window, int button, int action, int mods)
{
	if (TwEventMouseButtonGLFW(button, action))
		return;
	double x, y;
	glfwGetCursorPos(gWindow, &x, &y);


	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			GLuint index;
			glReadPixels(int(x), g_height - int(y), 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

			if (index > 0)
			{
				if (index <= mSet->mesh_models.size())
				{
					scene_light->not_click_light();
					selectedModel = index - 1;
					mSet->click_model(selectedModel);
					selectingModel = true;
					selectingLight = false;
				}
				else
				{
					mSet->not_click_model();
					scene_light->click_light();
					selectingModel = false;
					selectedModel = -1;
					selectingLight = true;
				}
			}
			else {
				selectingModel = false;
				selectedModel = -1;
				mSet->not_click_model();
				scene_light->not_click_light();
				selectingLight = false;
			}
		}
	}
}

void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (TwEventMouseWheelGLFW(int(yoffset)))
		return;
}

void posCursor(GLFWwindow* window, double x, double y)
{
	if (TwEventMousePosGLFW(x, y)) {
		lastX = x;
		lastY = y;
		return;
	}
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}
	GLfloat xoffset = GLfloat(x - lastX);
	GLfloat yoffset = GLfloat(lastY - y);
	lastX = x;
	lastY = y;
	if (activateCamera) {
		scene_camera->processMouseMovement(xoffset, yoffset);
	}
}

void charInput(GLFWwindow* window, unsigned int scanChar)
{
	if (TwEventCharGLFW(scanChar, GLFW_PRESS))
		return;
}

void dropPath(GLFWwindow* window, int count, const char** paths)
{
	//volumes->dropPath(count, paths);
}

void movement()
{
	if (keys[GLFW_KEY_W])
		scene_camera->processKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		scene_camera->processKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_D])
		scene_camera->processKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_A])
		scene_camera->processKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_Z])
		scene_camera->processKeyboard(UP, deltaTime);
	if (keys[GLFW_KEY_C])
		scene_camera->processKeyboard(DONW, deltaTime);
}

int negative_positive()
{
	if (rand() % 2)
		return 1;
	else
		return -1;
}

void generate_ortographic_cameras()
{
	double xPos, yPos, zPos;
	ScatteredMap *map;
	for (size_t i = 0; i < num_of_orto_cameras; i++)
	{
		xPos = halton_sequence(i, 2) + (3.0f * negative_positive());
		yPos = halton_sequence(i, 3) + (3.0f * negative_positive());
		zPos = halton_sequence(i, 7) + (3.0f * negative_positive());
		map = new ScatteredMap(glm::vec3(xPos, yPos, zPos));
		cameraPositions.push_back(map);
	}
}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void generateSamples()
{
	// Rango de n�meros flotantes [0:1]
	std::uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	glm::vec3 sample;
	GLfloat scale;

	for (size_t i = 0; i < num_of_samples_per_frag; ++i)
	{
		sample = glm::vec3(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		scale = GLfloat(i) / GLfloat(num_of_samples_per_frag);

		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		samples.push_back(sample);
	}
}

float calculate_c_phi(float ni)
{
	float C1 = 0.0f;
	if (ni < 1.0f)
		C1 = 0.919317f - 3.4793f * ni + 6.75335f * pow(ni, 2) - 7.80989f * pow(ni, 3) + 4.98554f * pow(ni, 4) - 1.36881f * pow(ni, 5);
	else
		C1 = -9.23372f + 22.2272f * ni - 20.9292f * pow(ni, 2) + 10.2291f * pow(ni, 3) - 2.54396f * pow(ni, 4) + 0.254913f * pow(ni, 5);
	return 1.0f / 4.0f * (1.0f - C1);
}

float calculate_c_e(float ni)
{
	float C2 = 0.0f;
	if (ni < 1.0f)
		C2 = 0.828421f - 2.62051f * ni + 3.36231f * pow(ni, 2) - 1.95284f * pow(ni, 3) + 0.236494f * pow(ni, 4) + 0.145787f * pow(ni, 5);
	else
		C2 = -1641.1f + 135.926f / pow(ni, 3) - 656.175f / pow(ni, 2) + 1376.53f / ni + 1213.67f * ni - 568.556f * pow(ni, 2) + 164.798f * pow(ni, 3) - 27.0181f * pow(ni, 4) + 1.91826f * pow(ni, 5);
	return 1.0f / 2.0f * (1.0f - C2);
}

void precalculate_values(mesh *m) {
	scattering_coeff_prime = scattering_coefficients[m->current_material] * (1.0f - m->asymmetry_param_g);
	attenuation_coeff = scattering_coefficients[m->current_material] + absorption_coefficients[m->current_material];
	albedo = scattering_coefficients[m->current_material] / attenuation_coeff;

	attenuation_coeff_prime = scattering_coeff_prime + absorption_coefficients[m->current_material];
	albedo_prime = scattering_coeff_prime / attenuation_coeff_prime;

	D = glm::vec3(1.0f) / (glm::vec3(3.0f) * attenuation_coeff_prime);
	effective_transport_coeff = sqrt(absorption_coefficients[m->current_material] / D);

	c_phi_1 = calculate_c_phi(m->refractive_index);
	c_phi_2 = calculate_c_phi(1 / m->refractive_index);
	c_e = calculate_c_e(m->refractive_index);

	A = (1.0f - c_e) / (2.0f * c_phi_1);
	de = 2.131f * D * sqrt(albedo_prime);
	zr = 3.0f * D;
}

bool initGlfw()
{
	g_width = 1200;
	g_height = 680;

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	gWindow = glfwCreateWindow(g_width, g_height, "Volume and Mesh Scattering", nullptr, nullptr);
	if (!gWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwSetFramebufferSizeCallback(gWindow, reshape);
	glfwMakeContextCurrent(gWindow);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	const GLFWvidmode * vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(gWindow, (vidMode->width - g_width) >> 1, (vidMode->height - g_height) >> 1);
	glfwSetFramebufferSizeCallback(gWindow, reshape);
	glfwMakeContextCurrent(gWindow);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetWindowSizeCallback(gWindow, reshape);
	glfwSetKeyCallback(gWindow, keyInput);
	glfwSetMouseButtonCallback(gWindow, click);
	glfwSetScrollCallback(gWindow, scroll);
	glfwSetCursorPosCallback(gWindow, posCursor);
	glfwSetCharCallback(gWindow, charInput);
	glfwSetDropCallback(gWindow, dropPath);

	return true;
}

bool initGlew()
{
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return false;
	else
	{
		std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

		glslProgram.loadShader("Shaders/program.vert", CGLSLProgram::VERTEX);
		glslProgram.loadShader("Shaders/program.frag", CGLSLProgram::FRAGMENT);
		glslGBuffer.loadShader("Shaders/gBuffer.vert", CGLSLProgram::VERTEX);
		glslGBuffer.loadShader("Shaders/gBuffer.frag", CGLSLProgram::FRAGMENT);
		glslGBufferP.loadShader("Shaders/gBufferPosition.vert", CGLSLProgram::VERTEX);
		glslGBufferP.loadShader("Shaders/gBufferPosition.frag", CGLSLProgram::FRAGMENT);
		glslScatteredMap.loadShader("Shaders/scatteredMap.vert", CGLSLProgram::VERTEX);
		glslScatteredMap.loadShader("Shaders/scatteredMap.frag", CGLSLProgram::FRAGMENT);

		glslProgram.create_link();
		glslGBuffer.create_link();
		glslGBufferP.create_link();
		glslScatteredMap.create_link();

		glslProgram.enable();
		glslProgram.addAttribute("position");
		glslProgram.addAttribute("normal");

		glslProgram.addUniform("view_matrix");
		glslProgram.addUniform("projection_matrix");
		glslProgram.addUniform("model_matrix");
		glslProgram.addUniform("lightPos");
		glslProgram.addUniform("view");
		glslProgram.addUniform("shinyBlinn");
		glslProgram.disable();

		glslGBuffer.enable();
		glslGBuffer.addAttribute("position");
		glslGBuffer.addAttribute("normal");

		glslGBuffer.addUniform("light_matrix");
		glslGBuffer.addUniform("model_matrix");
		glslGBuffer.disable();

		glslGBufferP.enable();
		glslGBufferP.addAttribute("position");
		glslGBufferP.addAttribute("texCoords");

		glslGBufferP.addUniform("model_matrix");
		glslGBufferP.addUniform("position_tex");
		glslGBufferP.disable();

		glslScatteredMap.enable();
		glslScatteredMap.addAttribute("position");
		glslScatteredMap.addAttribute("normal");

		glslScatteredMap.addUniform("camera_matrix");
		glslScatteredMap.addUniform("projection_matrix");
		glslScatteredMap.addUniform("model_matrix");

		glslScatteredMap.addUniform("asymmetry_param_g");
		glslScatteredMap.addUniform("light_pos");
		glslScatteredMap.addUniform("light_diff");
		glslScatteredMap.addUniform("n_samples");
		glslScatteredMap.addUniform("samples");
		glslScatteredMap.addUniform("g_position");
		glslScatteredMap.addUniform("g_normal");
		glslScatteredMap.addUniform("refractive_index");
		glslScatteredMap.addUniform("scattering_coeff");
		glslScatteredMap.addUniform("absorption_coeff");
		glslScatteredMap.addUniform("diffuse_reflectance");

		// Valores pre-calculados
		glslScatteredMap.addUniform("scattering_coeff_prime");
		glslScatteredMap.addUniform("attenuation_coeff");
		glslScatteredMap.addUniform("albedo");
		glslScatteredMap.addUniform("attenuation_coeff_prime");
		glslScatteredMap.addUniform("albedo_prime");
		glslScatteredMap.addUniform("D");
		glslScatteredMap.addUniform("effective_transport_coeff");
		glslScatteredMap.addUniform("c_phi_1");
		glslScatteredMap.addUniform("c_phi_2");
		glslScatteredMap.addUniform("c_e");
		glslScatteredMap.addUniform("A");
		glslScatteredMap.addUniform("de");
		glslScatteredMap.addUniform("zr");

		glslScatteredMap.disable();

		return true;
	}
}

bool initAntTweakBar()
{
	if (!TwInit(TW_OPENGL, NULL))
		return false;
	return true;
}

bool initScene()
{
	light_buffer *g_buffer;
	mesh *scene_model;

	num_of_lights = 1;
	num_of_orto_cameras = 16;
	num_of_samples_per_frag = 3 * num_of_orto_cameras;

	scene_light = new light();
	scene_model = new mesh();
	mSet = new meshSet();

	light_buffers = new light_buffers_set();

	for (size_t i = 0; i < num_of_lights; i++)
	{
		g_buffer = new light_buffer(g_width, g_height);
		light_buffers->array_of_buffers.push_back(g_buffer);
	}

	scene_camera = new camera(glm::vec3(0.0f, 0.0f, 8.0f));
	scene_model->load("Models/obj/bunny.obj");
	mSet->mesh_models.push_back(scene_model);
	generate_ortographic_cameras();
	generateSamples();
	return true;
}

void display()
{
	glm::mat4 projectionLightMat, viewLightMat, spaceLightMatrix1, spaceLightMatrix2, model_mat;

	view = scene_camera->getViewMatrix();
	projection = glm::perspective(scene_camera->zoom, (float)g_width / (float)g_height, 0.1f, 100.0f);

	for (int i = 0; i < num_of_lights; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, light_buffers->array_of_buffers[i]->g_buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glslGBuffer.enable();
		for (int j = 0; j < mSet->mesh_models.size(); j++)
		{
			model_mat = glm::mat4(1.0f);
			projectionLightMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 20.0f);
			viewLightMat = glm::lookAt(scene_light->translation, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			spaceLightMatrix1 = projectionLightMat * viewLightMat;

			model_mat = glm::translate(model_mat, mSet->mesh_models[j]->translation);
			model_mat = model_mat * glm::toMat4(mSet->mesh_models[j]->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[j]->scale));

			glUniformMatrix4fv(glslGBuffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
			glUniformMatrix4fv(glslGBuffer.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix1));

			glBindVertexArray(mSet->mesh_models[j]->vao);
			glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[j]->vertices.size());
			glBindVertexArray(0);
		}
		glslGBuffer.disable();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glslScatteredMap.enable();
	for (int i = 0; i < mSet->mesh_models.size(); i++)
	{
		unsigned int j = 0;
		//glBindFramebuffer(GL_FRAMEBUFFER, cameraPositions[i]->buffer);
		glStencilFunc(GL_ALWAYS, 1, -1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		precalculate_values(mSet->mesh_models[i]);
		model_mat = glm::mat4(1.0f);
		projectionLightMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 20.0f);
		viewLightMat = glm::lookAt(cameraPositions[i]->position, mSet->mesh_models[i]->center, glm::vec3(0.0f, 1.0f, 0.0f));
		spaceLightMatrix2 = projectionLightMat * viewLightMat;

		model_mat = glm::translate(model_mat, mSet->mesh_models[i]->translation);
		model_mat = model_mat * glm::toMat4(mSet->mesh_models[i]->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[i]->scale));

		glUniformMatrix4fv(glslScatteredMap.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glslScatteredMap.getLocation("camera_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix1));
		glUniformMatrix4fv(glslScatteredMap.getLocation("projection_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix1));
		glUniform1i(glslScatteredMap.getLocation("g_position"), 0);
		glUniform1i(glslScatteredMap.getLocation("g_normal"), 1);
		glUniform1i(glslScatteredMap.getLocation("n_samples"), num_of_samples_per_frag);
		glUniform3fv(glslScatteredMap.getLocation("samples"), num_of_samples_per_frag, glm::value_ptr(samples[0]));
		glUniform1f(glslScatteredMap.getLocation("asymmetry_param_g"), mSet->mesh_models[i]->asymmetry_param_g);
		glUniform1f(glslScatteredMap.getLocation("refractive_index"), mSet->mesh_models[i]->refractive_index);
		
		glUniform3f(glslScatteredMap.getLocation("scattering_coeff"), scattering_coefficients[mSet->mesh_models[i]->current_material].x, scattering_coefficients[mSet->mesh_models[i]->current_material].y, scattering_coefficients[mSet->mesh_models[i]->current_material].z);
		glUniform3f(glslScatteredMap.getLocation("absorption_coeff"), absorption_coefficients[mSet->mesh_models[i]->current_material].x, absorption_coefficients[mSet->mesh_models[i]->current_material].y, absorption_coefficients[mSet->mesh_models[i]->current_material].z);
		glUniform3f(glslScatteredMap.getLocation("diffuse_reflectance"), diffuse_reflectances[mSet->mesh_models[i]->current_material].x, diffuse_reflectances[mSet->mesh_models[i]->current_material].y, diffuse_reflectances[mSet->mesh_models[i]->current_material].z);

		glUniform3f(glslScatteredMap.getLocation("light_pos"), scene_light->translation.x, scene_light->translation.y, scene_light->translation.z);
		glUniform4f(glslScatteredMap.getLocation("light_diff"), 1.0f, 1.0f, 1.0f, 1.0f);

		// Valores pre-calculados
		glUniform3f(glslScatteredMap.getLocation("scattering_coeff_prime"), scattering_coeff_prime.x, scattering_coeff_prime.y, scattering_coeff_prime.z);
		glUniform3f(glslScatteredMap.getLocation("attenuation_coeff"), attenuation_coeff.x, attenuation_coeff.y, attenuation_coeff.z);
		glUniform3f(glslScatteredMap.getLocation("albedo"), albedo.x, albedo.y, albedo.z);
		glUniform3f(glslScatteredMap.getLocation("attenuation_coeff_prime"), attenuation_coeff_prime.x, attenuation_coeff_prime.y, attenuation_coeff_prime.z);
		glUniform3f(glslScatteredMap.getLocation("albedo_prime"), albedo_prime.x, albedo_prime.y, albedo_prime.z);
		glUniform3f(glslScatteredMap.getLocation("D"), D.x, D.y, D.z);
		glUniform3f(glslScatteredMap.getLocation("effective_transport_coeff"), effective_transport_coeff.x, effective_transport_coeff.y, effective_transport_coeff.z);
		glUniform1f(glslScatteredMap.getLocation("c_phi_1"), c_phi_1);
		glUniform1f(glslScatteredMap.getLocation("c_phi_2"), c_phi_2);
		glUniform1f(glslScatteredMap.getLocation("c_e"), c_e);
		glUniform1f(glslScatteredMap.getLocation("A"), A);
		glUniform3f(glslScatteredMap.getLocation("de"), de.x, de.y, de.z);
		glUniform3f(glslScatteredMap.getLocation("zr"), zr.x, zr.y, zr.z);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[j]->g_position);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[j]->g_normal);

		glBindVertexArray(mSet->mesh_models[i]->vao);
		glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[i]->vertices.size());
		glBindVertexArray(0);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glslScatteredMap.disable();

	//glEnable(GL_STENCIL_TEST);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//glslProgram.enable();
	//for (size_t i = 0; i < mSet->mesh_models.size(); i++)
	//{
	//	glStencilFunc(GL_ALWAYS, i + 1, -1);
	//	glUniform3f(glslProgram.getLocation("view"), scene_camera->position[0], scene_camera->position[1], scene_camera->position[2]);
	//	glUniform3f(glslProgram.getLocation("lightPos"), scene_light->translation.x, scene_light->translation.y, scene_light->translation.z);
	//	glUniform1f(glslProgram.getLocation("shinyBlinn"), mSet->mesh_models[i]->shininess);

	//	model_mat = glm::mat4(1.0f);
	//	model_mat = glm::translate(model_mat, mSet->mesh_models[i]->translation);
	//	model_mat = model_mat * glm::toMat4(mSet->mesh_models[i]->rotation);
	//	model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[i]->scale));

	//	glUniformMatrix4fv(glslProgram.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	//	glUniformMatrix4fv(glslProgram.getLocation("view_matrix"), 1, GL_FALSE, glm::value_ptr(view));
	//	glUniformMatrix4fv(glslProgram.getLocation("projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection));

	//	glBindVertexArray(mSet->mesh_models[i]->vao);
	//	glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[i]->vertices.size());
	//	glBindVertexArray(0);
	//}
	//glslProgram.disable();

	glStencilFunc(GL_ALWAYS, mSet->mesh_models.size() + 1, -1);
	scene_light->display(projection * view);

	glDisable(GL_STENCIL_TEST);

	glslGBufferP.enable();
	glm::mat4 model_gbuffer = glm::mat4(1.0f);
	model_gbuffer = glm::translate(model_gbuffer, glm::vec3(0.7, -0.7, -1.0));
	model_gbuffer = glm::scale(model_gbuffer, glm::vec3(0.3f));
	glUniformMatrix4fv(glslGBufferP.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_gbuffer));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[0]->g_position);
	render_quad();
	glslGBufferP.disable();
}

void destroy()
{
	glfwDestroyWindow(gWindow);
	glfwTerminate();
}

int main()
{
	if (!initGlfw() || !initGlew() || !initAntTweakBar() || !initScene())
		return EXIT_FAILURE;

	glEnable(GL_DEPTH_TEST);
	reshape(gWindow, g_width, g_height);

	while (!glfwWindowShouldClose(gWindow))
	{
		GLfloat currentFrame = float(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		movement();
		display();
		TwDraw();
		scene_light->update_interface();
		mSet->update_interface(selectedModel);
		glfwSwapBuffers(gWindow);
	}

	destroy();

	return EXIT_SUCCESS;
}