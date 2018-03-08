#include "Main.h"
#include "Model.h"

GLFWwindow *gWindow;
int gWidth, gHeight;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f;
bool keys[1024], keysPressed[1024];
glm::mat4 projection, view, model_view;
TwBar *menuTW, *modelTW;
camera *sceneCamera;

float shinyBlinn = 128.0, scaleT = 5.00, ejeX = 1.51, ejeY = 0.26, ejeZ = -1.33, ejeXL = 0.23, ejeYL = 1.18, ejeZL = 0.0;
float rotacionPrincipal[] = { 0.0f, 0.0f, 0.0f, 1.0f };
vector<model> models; //Todos los modelos irán en este vector
model m;
CGLSLProgram glslProgram;
int selectedModel = 0;

glm::mat4 project_mat; //Matriz de Proyección
glm::mat4 view_mat; //Matriz de View
glm::vec3 eye(0.0f, 0.0f, 2.0f); // Ojo
float lightDirection[] = { -1.31, -0.12, 1.10 };

void reshape(GLFWwindow *window, int width, int height)
{
	gWidth = width;
	gHeight = height;
	glViewport(0, 0, gWidth, gHeight);
	TwWindowSize(width, height);
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
	}
}

void click(GLFWwindow* window, int button, int action, int mods)
{
	if (TwEventMouseButtonGLFW(button, action))
		return;
}

void posCursor(GLFWwindow* window, double x, double y)
{
	if (TwEventMousePosGLFW(int(x), int(y)))
		return;
}

void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	//volumes->scrollVolume(yoffset);
}

void charInput(GLFWwindow* window, unsigned int scanChar)
{
	if (TwEventCharGLFW(scanChar, GLFW_PRESS))
		return;
}

void TW_CALL exit(void *clientData) {
	exit(1);
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
	glfwSetCursorPosCallback(gWindow, posCursor);
	glfwSetCharCallback(gWindow, charInput);
	glfwSetScrollCallback(gWindow, scroll);
	glfwSetDropCallback(gWindow, dropPath);

	return true;
}

bool initGlew()
{
	TwInit(TW_OPENGL, NULL);
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
		glslProgram.create_link();
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

		return true;
	}
}

bool initAntTweakBar()
{

	menuTW = TwNewBar("Menú");
	TwDefine("Menú visible=false size='270 80' position='20 20' color='128 0 0' label='Volume and Mesh Scattering'");
	TwAddButton(menuTW, "exit", exit, NULL, " label='Salir' key=Esc");

	modelTW = TwNewBar("Figura");
	TwWindowSize(200, 400);
	TwDefine("Figura visible=true size='270 520' position='20 20' color='128 0 0' label='Objeto'");

	TwAddVarRW(modelTW, "scale", TW_TYPE_FLOAT, &scaleT, "min=0.01 step=0.01 label='Escalar' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeX", TW_TYPE_FLOAT, &ejeX, "step=0.01 label='Traslación x' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeY", TW_TYPE_FLOAT, &ejeY, "step=0.01 label='Traslación y' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeZ", TW_TYPE_FLOAT, &ejeZ, "step=0.01 label='Traslación z' group='Transformaciones'");
	TwAddVarRW(modelTW, "rotation", TW_TYPE_QUAT4F, &rotacionPrincipal, " label='Rotación' opened=true group='Transformaciones'");
	
	TwAddVarRW(modelTW, "ejeXL", TW_TYPE_FLOAT, &ejeXL, "step=0.01 label='x' group='Trasladar luz' group='Luz'");
	TwAddVarRW(modelTW, "ejeYL", TW_TYPE_FLOAT, &ejeYL, "step=0.01 label='y' group='Trasladar luz' group='Luz'");
	TwAddVarRW(modelTW, "ejeZL", TW_TYPE_FLOAT, &ejeZL, "step=0.01 label='z' group='Trasladar luz' group='Luz'");
	TwAddVarRW(modelTW, "BrilloBlinn", TW_TYPE_FLOAT, &shinyBlinn, "min=1.0 max=400.0 step=1.0 label='Shininess' group='Luz'");

	TwAddButton(modelTW, "exitF", exit, NULL, " label='Salir' key=Esc");

	return true;
}

void movement()
{
	if (keys[GLFW_KEY_W])
		sceneCamera->processKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		sceneCamera->processKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_D])
		sceneCamera->processKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_A])
		sceneCamera->processKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_Z])
		sceneCamera->processKeyboard(UP, deltaTime);
	if (keys[GLFW_KEY_C])
		sceneCamera->processKeyboard(DONW, deltaTime);
}

void initScene()
{
	sceneCamera = new camera(glm::vec3(1.0f, 0.0f, 6.0f));	
}

void display()
{
	glm::mat4 VP;
	glm::vec3 viewPos;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	view = sceneCamera->getViewMatrix();
	projection = glm::perspective(sceneCamera->zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);
	VP = projection * view;
	viewPos = sceneCamera->position;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//Colocar este código en una función setCurrentValues(selectedModel)
	models[selectedModel].rotacion[0] = rotacionPrincipal[0];
	models[selectedModel].rotacion[1] = rotacionPrincipal[1];
	models[selectedModel].rotacion[2] = rotacionPrincipal[2];
	models[selectedModel].rotacion[3] = rotacionPrincipal[3];

	models[selectedModel].ejeX = ejeX;
	models[selectedModel].ejeY = ejeY;
	models[selectedModel].ejeZ = ejeZ;

	models[selectedModel].scaleT = scaleT;
	models[selectedModel].shinyBlinn = shinyBlinn;

	for (int i = 0; i<models.size(); i++) {

		//glStencilFunc(GL_ALWAYS, i, -1);
		glslProgram.enable();
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");

		glUniform3f(view_loc, sceneCamera->position[0], sceneCamera->position[1], sceneCamera->position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, ejeZL);
		glUniform1f(shinyBlinn_loc, models[i].shinyBlinn);

		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = glm::lookAt(eye, norm, up);
		view_mat = sceneCamera->getViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[i].ejeX, models[i].ejeY, models[i].ejeZ);
		model_mat = model_mat * m.rotacion_en_matriz(models[i].rotacion[0], models[i].rotacion[1], models[i].rotacion[2], models[i].rotacion[3]);
		model_mat = model_mat * m.scale_en_matriz(models[i].scaleT);

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
		project_mat = glm::perspective(sceneCamera->zoom, (float)gWidth / (float)gHeight, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));

		glBindBuffer(GL_ARRAY_BUFFER, models[i].vbo);
		//Se bindean los vértices, normales y coordenadas de texturas
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + (models[i].coord_texturas.size() * sizeof(float))));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, models[i].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		glPushMatrix();
		glPopMatrix();

	}
}

void destroy()
{
	glfwDestroyWindow(gWindow);
	glfwTerminate();
}

int main()
{
	if (!initGlfw() || !initGlew() || !initAntTweakBar())
		return EXIT_FAILURE;

	initScene();
	reshape(gWindow, gWidth, gHeight);
	m.read_obj("Models/obj/cornell-box.obj");
	while (!glfwWindowShouldClose(gWindow))
	{
		GLfloat currentFrame = float(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		movement();
		display();
		TwDraw();
		glfwSwapBuffers(gWindow);
	}

	destroy();

	return EXIT_SUCCESS;
}