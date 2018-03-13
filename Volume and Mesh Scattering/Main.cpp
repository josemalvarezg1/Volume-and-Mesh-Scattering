#include "Main.h"
#include "Model.h"
#include "Halton.h"
#include "ScatteredMap.h"

GLFWwindow *gWindow;
int gWidth, gHeight;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f, lastX = 600, lastY = 340;
bool keys[1024], keysPressed[1024], selectingModel = false, selectingLight = false, firstMouse = true, activateCamera = false;
std::vector<ScatteredMap*> cameraPositions;
glm::mat4 projection, view, model;

light *scene_light;
camera *scene_camera;
meshSet *mSet;

CGLSLProgram glslProgram, glslGBuffer, glslGBufferP;
int selectedModel = -1;
glm::vec3 eye(0.0f, 0.0f, 2.0f); // Ojo

GLuint quadVAO, quadVBO;
unsigned int gBuffer, gPosition, gNormal;
unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

void create_gbuffer()
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, gWidth, gHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gPosition);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, gWidth, gHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gNormal);

	glDrawBuffers(2, attachments);
}

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
	gWidth = width;
	gHeight = height;

	scene_light->light_interface->reshape();
	TwWindowSize(width, height);

	glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
	glViewport(0, 0, gWidth, gHeight);
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
			GLint index;
			glReadPixels(x, gHeight - y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

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
	if (TwEventMouseWheelGLFW(yoffset))
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
	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;
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

bool initGlfw()
{
	gWidth = 1200;
	gHeight = 680;

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	gWindow = glfwCreateWindow(gWidth, gHeight, "Volume and Mesh Scattering", nullptr, nullptr);
	if (!gWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwSetFramebufferSizeCallback(gWindow, reshape);
	glfwMakeContextCurrent(gWindow);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	const GLFWvidmode * vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(gWindow, (vidMode->width - gWidth) >> 1, (vidMode->height - gHeight) >> 1);
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

		glslProgram.create_link();
		glslGBuffer.create_link();
		glslGBufferP.create_link();

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

		return true;
	}
}

bool initAntTweakBar()
{
	if (!TwInit(TW_OPENGL, NULL))
		return false;
	return true;
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

void generateOrtographicCameras(int cameraCount) {
	double xPos, yPos, zPos;
	ScatteredMap *map;
	for (int i = 0; i < cameraCount; i++)
	{
		// Revisar esto
		srand(13);
		int factor = -1;
		if (((float)rand()) / RAND_MAX * 100.0 - 50.0 > 0) factor = 1;
		else factor = -1;
		xPos = Halton_Seq(i, 2) + 2.0 * factor;
		if (((float)rand()) / RAND_MAX * 100.0 - 50.0 > 0) factor = 1;
		else factor = -1;
		yPos = Halton_Seq(i, 3) + 2.0 * factor;
		if (((float)rand()) / RAND_MAX * 100.0 - 50.0 > 0) factor = 1;
		else factor = -1;
		zPos = Halton_Seq(i, 7) + 2.0 * factor;
		map = new ScatteredMap(glm::vec3(xPos, yPos, zPos));
		cameraPositions.push_back(map);
	}
}

bool initScene()
{
	mesh *scene_model;

	scene_light = new light();
	scene_model = new mesh();
	mSet = new meshSet();
	scene_camera = new camera(glm::vec3(0.0f, 0.0f, 8.0f));
	scene_model->load("Models/obj/mickey.obj");
	mSet->mesh_models.push_back(scene_model);
	create_gbuffer();
	generateOrtographicCameras(16);
	return true;
}

void display()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = scene_camera->getViewMatrix();
	projection = glm::perspective(scene_camera->zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);

	glslGBuffer.enable();
	glm::mat4 projectionLightMat, viewLightMat, spaceLightMatrix, model_mat;

	for (int i = 0; i < mSet->mesh_models.size(); i++)
	{
		model_mat = glm::mat4(1.0f);
		projectionLightMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 20.0f);
		viewLightMat = glm::lookAt(scene_light->translation, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		spaceLightMatrix = projectionLightMat * viewLightMat;

		model_mat = glm::translate(model_mat, mSet->mesh_models[i]->translation);
		model_mat = model_mat * glm::toMat4(mSet->mesh_models[i]->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[i]->scale));

		glUniformMatrix4fv(glslGBuffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glslGBuffer.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix));

		glBindVertexArray(mSet->mesh_models[i]->vao);
		glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[i]->vertices.size());
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glslGBuffer.disable();

	glslGBuffer.enable();
	for (size_t i = 0; i < cameraPositions.size(); i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, cameraPositions[i]->buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model_mat = glm::mat4(1.0f);
		projectionLightMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 20.0f);
		viewLightMat = glm::lookAt(cameraPositions[i]->position, mSet->mesh_models[0]->center, glm::vec3(0.0f, 1.0f, 0.0f));
		spaceLightMatrix = projectionLightMat * viewLightMat;

		model_mat = glm::translate(model_mat, mSet->mesh_models[0]->translation);
		model_mat = model_mat * glm::toMat4(mSet->mesh_models[0]->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[0]->scale));

		glUniformMatrix4fv(glslGBuffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glslGBuffer.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix));

		glBindVertexArray(mSet->mesh_models[0]->vao);
		glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[0]->vertices.size());
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glslGBuffer.disable();

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glslProgram.enable();
	for (size_t i = 0; i < mSet->mesh_models.size(); i++)
	{
		glStencilFunc(GL_ALWAYS, i + 1, -1);
		glUniform3f(glslProgram.getLocation("view"), scene_camera->position[0], scene_camera->position[1], scene_camera->position[2]);
		glUniform3f(glslProgram.getLocation("lightPos"), scene_light->translation.x, scene_light->translation.y, scene_light->translation.z);
		glUniform1f(glslProgram.getLocation("shinyBlinn"), mSet->mesh_models[i]->shininess);

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, mSet->mesh_models[i]->translation);
		model_mat = model_mat * glm::toMat4(mSet->mesh_models[i]->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(mSet->mesh_models[i]->scale));

		glUniformMatrix4fv(glslProgram.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glslProgram.getLocation("view_matrix"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glslProgram.getLocation("projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(mSet->mesh_models[i]->vao);
		glDrawArrays(GL_TRIANGLES, 0, mSet->mesh_models[i]->vertices.size());
		glBindVertexArray(0);
	}
	glslProgram.disable();

	glStencilFunc(GL_ALWAYS, mSet->mesh_models.size() + 1, -1);
	scene_light->display(projection * view);

	glDisable(GL_STENCIL_TEST);

	glslGBufferP.enable();
	glm::mat4 model_gbuffer = glm::mat4(1.0f);
	model_gbuffer = glm::translate(model_gbuffer, glm::vec3(0.7, -0.7, -1.0));
	model_gbuffer = glm::scale(model_gbuffer, glm::vec3(0.3f));
	glUniformMatrix4fv(glslGBufferP.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_gbuffer));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cameraPositions[15]->texture);

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
	reshape(gWindow, gWidth, gHeight);

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