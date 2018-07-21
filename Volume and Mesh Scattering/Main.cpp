#include "Main.h"
#define INITIAL_NUM_OF_CAMERAS 6

GLFWwindow *g_window;
int g_width, g_height;
GLfloat delta_time = 0.0f, last_frame = 0.0f, current_frame;
GLdouble last_x = 600.0, last_y = 340.0;
bool keys[1024], keys_pressed[1024], selecting_model = false, selecting_light = false, first_mouse = true, activate_camera = false, change_light = false, selecting_volume = false, scattering_model = true, scattering_volume = true, gradient_volume = false, model_center = true, last_model_center = true, specular_flag = false, volume_transparent = false;
glm::mat4 projection, view, model_mat;
texture_t current_texture_type;

light* scene_light;
camera *scene_camera;
model_m current_model = Bunny;
volume_v current_volume = Bucky;
mesh* scene_cornell;
interface_menu *scene_interface;
interface_function *transfer_function;
volume_render *volumes;
model *translucent_model;

CGLSLProgram glsl_cornell;
int selected_model = -1;

void update_interface_menu()
{
	if (selecting_volume)
		scene_interface->update_position();
	else
		scene_interface->update_width(g_width);
	if (translucent_model->num_of_ortho_cameras != scene_interface->num_of_cameras)
	{
		translucent_model->scene_model->change_values = true;
		scene_interface->camera_selected = 0;
	}
	current_texture_type = scene_interface->current_texture_type;
	translucent_model->num_of_ortho_cameras = scene_interface->num_of_cameras;
	scene_interface->set_max_values(translucent_model->num_of_ortho_cameras - 1);
	translucent_model->selected_camera = scene_interface->camera_selected;
	if (last_model_center != model_center) 
	{
		translucent_model->scene_model->change_values = true;
		last_model_center = model_center;
	}
	if (current_model != scene_interface->current_model) 
	{
		translucent_model->scene_model->~mesh();
		current_model = scene_interface->current_model;
		translucent_model->scene_model = new mesh();
		switch (current_model) {
			case Bunny:
				translucent_model->scene_model->load("Models/obj/bunny.obj");
				break;
			case Hebe:
				translucent_model->scene_model->load("Models/obj/hebe.obj");
				translucent_model->scene_model->scale = 10.0f;
				translucent_model->scene_model->rotation = glm::quat(0.7f, -0.7f, 0.0f, 0.0f);
				break;
			case Buddha:
				translucent_model->scene_model->load("Models/obj/buddha.obj");
				translucent_model->scene_model->scale = 3.25f;
				translucent_model->scene_model->rotation = glm::quat(0.7f, -0.7f, 0.0f, 0.0f);
				break;
			case Dragon:
				translucent_model->scene_model->load("Models/obj/dragon.obj");
				translucent_model->scene_model->scale = 3.25f;
				translucent_model->scene_model->rotation = glm::quat(0.7f, -0.7f, 0.0f, 0.0f);
				break;
			case Esfera:
				translucent_model->scene_model->load("Models/obj/sphere.obj");
				translucent_model->scene_model->scale = 2.5f;
				break;
		}		
		scene_light->not_click_light();
		volumes->volume_interface->hide();
		translucent_model->scene_model->click_model();
		selecting_model = true;
		selecting_light = false;
		selecting_volume = false;
		transfer_function->hide = true;
		translucent_model->scene_model->change_values = true;
	}
	if (current_volume != scene_interface->current_volume)
	{
		current_volume = scene_interface->current_volume;
		volumes->change_values = true;
		volumes->volumes[current_volume]->translation = volumes->volumes[volumes->index_select]->translation;
		volumes->volumes[current_volume]->rotation = volumes->volumes[volumes->index_select]->rotation;
		volumes->volumes[current_volume]->escalation = volumes->volumes[volumes->index_select]->escalation;
		volumes->index_select = current_volume;
		scene_light->not_click_light();
		volumes->volume_interface->show();
		translucent_model->scene_model->not_click_model();
		selecting_model = false;
		selecting_light = false;
		selecting_volume = true;
		transfer_function->hide = false;
		translucent_model->scene_model->change_values = false;
	}
}

void click_interface_menu()
{
	scene_interface->show();
	scene_interface->num_of_cameras = INITIAL_NUM_OF_CAMERAS;
	scene_interface->camera_selected = translucent_model->selected_camera;
}

void reshape(GLFWwindow *window, int width, int height)
{
	g_width = std::max(width, 1);
	g_height = std::max(height, 1);

	volumes->resize_screen(glm::vec2(g_width, g_height));
	translucent_model->scene_model->model_interface->reshape(g_width, g_height);
	scene_interface->update_width(g_width);

	translucent_model->light_buffers->update_g_buffer(g_width, g_height, 1);
	scene_light->light_interface->reshape(g_width, g_height);
	translucent_model->scene_model->change_values = true;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	g_width = std::max(width, 16);
	g_height = std::max(height, 16);
	translucent_model->scattered_maps->update_scattered_map(g_width, g_height, translucent_model->num_of_ortho_cameras);
	glViewport(0, 0, g_width, g_height);
}

void key_input(GLFWwindow *window, int key, int scan_code, int action, int mods)
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
			keys_pressed[key] = false;
		}

		if (key == GLFW_KEY_T && (action == GLFW_PRESS))
		{
			activate_camera = !activate_camera;
			if (activate_camera)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				scene_interface->hide();
				if (selecting_model)
					translucent_model->scene_model->not_click_model();
				if (selecting_light)
					scene_light->not_click_light();
				if (selecting_volume) {
					transfer_function->hide = true;
					volumes->volume_interface->hide();
				}
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				scene_interface->show();
				if (selecting_model)
					translucent_model->scene_model->click_model();
				if (selecting_light)
					scene_light->click_light();
				if (selecting_volume) {
					transfer_function->hide = false;
					volumes->volume_interface->show();
				}
			}
		}
		if (keys[GLFW_KEY_DELETE])
		{
			transfer_function->delete_point();
			volumes->change_values = true;
		}
			
		if (keys[GLFW_KEY_M])
			transfer_function->movable = true;
		if (keys[GLFW_KEY_N])
			transfer_function->movable = false;
	}
}

void click(GLFWwindow* window, int button, int action, int mods)
{
	if (TwEventMouseButtonGLFW(button, action))
		return;
	double x, y;
	glfwGetCursorPos(g_window, &x, &y);
	view = scene_camera->get_view_matrix();
	projection = glm::perspective(scene_camera->zoom, (float)g_width / (float)g_height, 0.1f, 100.0f);
	if (action == GLFW_PRESS && !activate_camera)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			GLuint index;
			glReadPixels(int(x), g_height - int(y), 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

			if (index > 0)
			{
				if (index <= 1)
				{
					scene_light->not_click_light();
					volumes->volume_interface->hide();
					selected_model = index - 1;
					translucent_model->scene_model->click_model();
					selecting_model = true;
					selecting_light = false;
					selecting_volume = false;
					transfer_function->hide = true;
				}
				else
				{
					translucent_model->scene_model->not_click_model();
					volumes->volume_interface->hide();
					scene_light->click_light();
					selecting_model = false;
					selected_model = -1;
					selecting_light = true;
					selecting_volume = false;
					transfer_function->hide = true;
				}
			}
			else {
				selecting_model = false;
				selected_model = -1;
				translucent_model->scene_model->not_click_model();
				scene_light->not_click_light();
				selecting_light = false;
			}
			if (transfer_function->click_transfer_f(x, y, g_width, g_height))
			{
				volumes->update_transfer_function(transfer_function->get_color_points());
				return;
			}
			if (volumes->click_volume(x, y, projection, view, scene_camera->position, false)) {
				selecting_volume = true;
				transfer_function->hide = false;
				volumes->volume_interface->show();
				translucent_model->scene_model->not_click_model();
				scene_light->not_click_light();
				return;
			}
		}
		else
		{
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
			{
				if (volumes->click_volume(x, y, projection, view, scene_camera->position, true))
					return;
			}
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			transfer_function->disable_select();
			volumes->disable_select();
		}
		else
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				volumes->disable_select();
	}
}

void scroll(GLFWwindow* window, double x_offset, double y_offset)
{
	if (TwEventMouseWheelGLFW(int(y_offset)))
		return;
}

void pos_cursor(GLFWwindow* window, double x, double y)
{
	if (TwEventMousePosGLFW((int) x, (int) y))
	{
		last_x = x;
		last_y = y;
		return;
	}
	if (first_mouse)
	{
		last_x = x;
		last_y = y;
		first_mouse = false;
	}
	GLfloat x_offset, y_offset;
	x_offset = GLfloat(x - last_x);
	y_offset = GLfloat(last_y - y);
	last_x = x;
	last_y = y;
	if (activate_camera)
		scene_camera->process_mouse_movement(x_offset, y_offset);

	if (transfer_function->poscursor_transfer_f(x, y, g_width, g_height))
	{
		volumes->update_transfer_function(transfer_function->get_color_points());
		return;
	}
}

void char_input(GLFWwindow* window, unsigned int scan_char)
{
	if (TwEventCharGLFW(scan_char, GLFW_PRESS))
		return;
}

void drop_path(GLFWwindow* window, int count, const char** paths)
{
	volumes->drop_path(count, paths, g_width, g_height);
}

void movement()
{
	if (keys[GLFW_KEY_W])
		scene_camera->process_keyboard(FORWARD, delta_time);
	if (keys[GLFW_KEY_S])
		scene_camera->process_keyboard(BACKWARD, delta_time);
	if (keys[GLFW_KEY_D])
		scene_camera->process_keyboard(RIGHT, delta_time);
	if (keys[GLFW_KEY_A])
		scene_camera->process_keyboard(LEFT, delta_time);
	if (keys[GLFW_KEY_Z])
		scene_camera->process_keyboard(UP, delta_time);
	if (keys[GLFW_KEY_C])
		scene_camera->process_keyboard(DOWN, delta_time);
}

bool init_glfw()
{
	g_width = 1200;
	g_height = 680;

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	g_window = glfwCreateWindow(g_width, g_height, "Volume and Mesh Scattering", nullptr, nullptr);
	if (!g_window)
	{
		glfwTerminate();
		return false;
	}

	glfwSetFramebufferSizeCallback(g_window, reshape);
	glfwMakeContextCurrent(g_window);
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	const GLFWvidmode * vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(g_window, (vidMode->width - g_width) >> 1, (vidMode->height - g_height) >> 1);
	glfwSetFramebufferSizeCallback(g_window, reshape);
	glfwMakeContextCurrent(g_window);
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetWindowSizeCallback(g_window, reshape);
	glfwSetKeyCallback(g_window, key_input);
	glfwSetMouseButtonCallback(g_window, click);
	glfwSetScrollCallback(g_window, scroll);
	glfwSetCursorPosCallback(g_window, pos_cursor);
	glfwSetCharCallback(g_window, char_input);
	glfwSetDropCallback(g_window, drop_path);

	return true;
}

bool init_glew()
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

		glsl_cornell.loadShader("Shaders/cornell.vert", CGLSLProgram::VERTEX);
		glsl_cornell.loadShader("Shaders/cornell.frag", CGLSLProgram::FRAGMENT);
		
		glsl_cornell.create_link();		

		glsl_cornell.enable();
		glsl_cornell.addAttribute("position");
		glsl_cornell.addAttribute("normal");
		glsl_cornell.addUniform("MVP");
		glsl_cornell.addUniform("light_pos");
		glsl_cornell.addUniform("model_matrix");
		glsl_cornell.disable();

		return true;
	}
}

bool init_ant_tweak_bar()
{
	if (!TwInit(TW_OPENGL, NULL))
		return false;
	return true;
}

bool init_scene()
{	
	translucent_model = new model(g_width, g_height);
	scene_interface = interface_menu::instance();
	click_interface_menu();
	current_texture_type = Scattered_Map;
	scene_cornell = new mesh();

	transfer_function = new interface_function();
	volumes = new volume_render(g_width, g_height);
	volumes->update_transfer_function(transfer_function->get_color_points());
	transfer_function->hide = true;

	scene_light = new light();
	scene_light->translation = glm::vec3(0.0, 6.0f, 5.0f);	

	scene_camera = new camera(glm::vec3(0.0f, 0.0f, 14.5f));

	scene_cornell->load("Models/obj/cornell.obj");
	scene_cornell->scale = 15.0f;
	scene_cornell->translation.x = 0.5f;

	const char** paths = new const char*[1];
	paths[0] = "Models\\raw\\bucky_32x32x32_8.raw";
	volumes->drop_path(1, paths, g_width, g_height);
	paths[0] = "Models\\raw\\bonsai_256x256x256_8.raw";
	volumes->drop_path(1, paths, g_width, g_height);
	paths[0] = "Models\\raw\\engine_256x256x256_8.raw";
	volumes->drop_path(1, paths, g_width, g_height);
	volumes->index_select = 0;

	return true;
}

void display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	view = scene_camera->get_view_matrix();
	projection = glm::perspective(scene_camera->zoom, (float)g_width / (float)g_height, 0.1f, 100.0f);

	translucent_model->display(projection, view, g_width, g_height, scene_interface->shininess, scattering_model,
							   change_light, model_center, specular_flag, scene_light->translation, scene_light->diffuse_comp,
							   scene_light->ambient_comp, scene_light->specular_comp, scene_camera->position);
	glStencilFunc(GL_ALWAYS, 2, -1);
	scene_light->display(projection * view);

	glDisable(GL_STENCIL_TEST);

	glsl_cornell.enable();
	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, scene_cornell->translation);
	model_mat = model_mat * glm::toMat4(scene_cornell->rotation);
	model_mat = glm::scale(model_mat, glm::vec3(scene_cornell->scale));

	glUniformMatrix4fv(glsl_cornell.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model_mat));
	glUniformMatrix4fv(glsl_cornell.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniform3fv(glsl_cornell.getLocation("light_pos"), 1, glm::value_ptr(scene_light->translation));

	glBindVertexArray(scene_cornell->vao);
	glDrawArrays(GL_TRIANGLES, 0, scene_cornell->vertices.size());
	glBindVertexArray(0);
	glsl_cornell.disable();

	glDisable(GL_DEPTH_TEST);

	if (change_light)
		volumes->change_values = true;
	volumes->display(projection, view, scene_camera->position, scene_light, transfer_function, scattering_volume, gradient_volume, volume_transparent);
	transfer_function->display();

	if (!selecting_volume && scattering_model)
		translucent_model->display_g_buffer(current_texture_type);
}

void destroy()
{
	glfwDestroyWindow(g_window);
	glfwTerminate();
	transfer_function->~interface_function();
}

int main()
{
	if (!init_glfw() || !init_glew() || !init_ant_tweak_bar() || !init_scene())
		return EXIT_FAILURE;

	glEnable(GL_DEPTH_TEST);
	reshape(g_window, g_width, g_height);
	translucent_model->num_of_ortho_cameras = INITIAL_NUM_OF_CAMERAS;

	while (!glfwWindowShouldClose(g_window))
	{
		current_frame = float(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		glfwPollEvents();
		movement();
		transfer_function->update_coords();
		display();
		TwDraw();
		change_light = scene_light->update_interface();
		translucent_model->scene_model->update_interface();
		volumes->update_interface();
		update_interface_menu();
		glfwSwapBuffers(g_window);
	}

	destroy();

	return EXIT_SUCCESS;
}