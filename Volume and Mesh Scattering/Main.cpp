#include "Main.h"

GLFWwindow *gWindow;
int gWidth, gHeight;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f;
bool keys[1024], keysPressed[1024];
glm::mat4 projection, view, model;

camera *sceneCamera;

void rescale(GLFWwindow *window, int width, int height)
{
	gWidth = width;
	gHeight = height;
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

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	gWindow = glfwCreateWindow(gWidth, gHeight, "Volume and Mesh Scattering", NULL, NULL);

	if (!gWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(gWindow);

	const GLFWvidmode * vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(gWindow, (vidMode->width - gWidth) >> 1, (vidMode->height - gHeight) >> 1);

	glfwSetWindowSizeCallback(gWindow, rescale);
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
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return false;
	else
	{
		std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

		return true;
	}
}

bool initAntTweakBar()
{
	if (!TwInit(TW_OPENGL_CORE, NULL))
		return false;
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
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	view = sceneCamera->getViewMatrix();
	projection = glm::perspective(sceneCamera->zoom, (float)gWidth / (float)gHeight, 0.1f, 100.0f);
	VP = projection * view;
	viewPos = sceneCamera->position;
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
	rescale(gWindow, gWidth, gHeight);

	while (!glfwWindowShouldClose(gWindow))
	{
		GLfloat currentFrame = float(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		movement();
		display();
		//TwDraw();
		glfwSwapBuffers(gWindow);
	}

	destroy();

	return EXIT_SUCCESS;
}