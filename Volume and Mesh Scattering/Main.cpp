#include "Main.h"

GLFWwindow *g_window;
int g_width, g_height;
GLuint num_of_lights, num_of_ortho_cameras, num_of_samples_per_frag, selected_light;
GLfloat delta_time = 0.0f, last_frame = 0.0f, current_frame;
GLdouble last_x = 600.0, last_y = 340.0;
bool keys[1024], keys_pressed[1024], selecting_model = false, selecting_light = false, first_mouse = true, activate_camera = false, change_light = false, selecting_volume = false;
scattered_map *scattered_maps;
halton *halton_generator;
glm::mat4 projection, view, model;

std::vector<light*> scene_lights;
camera *scene_camera;
mesh *scene_model;
light_buffer *light_buffers;
materials_set *materials;
interface_function *transfer_funtion;
volume_render *volumes;

CGLSLProgram glsl_g_buffer, glsl_g_buffer_plane, glsl_scattered_map, glsl_mipmaps, glsl_blending, glsl_test;
int selected_model = -1;
GLuint quad_vao, quad_vbo, texture_vao, texture_vbo;

unsigned int g_buffer;
unsigned int g_out, g_out_prev, vol_ilum;
unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

void create_gbuffer()
{
	glGenFramebuffers(1, &g_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glGenTextures(1, &g_out);
	glBindTexture(GL_TEXTURE_2D, g_out);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_width, g_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_out, 0);

	glGenTextures(1, &g_out_prev);
	glBindTexture(GL_TEXTURE_2D, g_out_prev);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_width, g_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_out_prev, 0);

	glDrawBuffers(2, attachments);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &vol_ilum);
	glBindTexture(GL_TEXTURE_3D, vol_ilum);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, volumes->volumes[0]->width, volumes->volumes[0]->height, volumes->volumes[0]->depth);
	glBindTexture(GL_TEXTURE_3D, 0);

}

void create_quad_light_volume()
{
	GLfloat quad_data[] = {
		-0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  1.0f, 0.0f,
	};
	glGenVertexArrays(1, &texture_vao);
	glGenBuffers(1, &texture_vbo);
	glBindVertexArray(texture_vao);
	glBindBuffer(GL_ARRAY_BUFFER, texture_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), &quad_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void render_quad()
{
	if (quad_vao == 0)
	{
		GLfloat quad_vertices[] =
		{
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void reshape(GLFWwindow *window, int width, int height)
{
	g_width = max(width, 1);
	g_height = max(height, 1);

	volumes->resize_screen(glm::vec2(g_width, g_height));
	scene_model->model_interface->reshape(g_width, g_height);

	for (size_t i = 0; i < num_of_lights; i++) {
		light_buffers->update_g_buffer(g_width, g_height, num_of_lights);
		scene_lights[i]->light_interface->reshape(g_width, g_height);
	}

	scene_model->change_values = true;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	scattered_maps->update_scattered_map(g_width, g_height, num_of_ortho_cameras);
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
				if (selecting_model)
					scene_model->not_click_model();
				if (selecting_light) {
					for (int i = 0; i < num_of_lights; i++)
						scene_lights[i]->not_click_light();
				}
				if (selecting_volume) {
					transfer_funtion->hide = true;
					volumes->volume_interface->hide();
				}
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (selecting_model)
					scene_model->click_model();
				if (selecting_light)
					scene_lights[selected_light]->click_light();
				if (selecting_volume) {
					transfer_funtion->hide = false;
					volumes->volume_interface->show();
				}
			}
		}
		if (keys[GLFW_KEY_DELETE])
			transfer_funtion->delete_point();
		if (keys[GLFW_KEY_M])
			transfer_funtion->movable = true;
		if (keys[GLFW_KEY_N])
			transfer_funtion->movable = false;
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
					for (int i = 0; i < num_of_lights; i++)
						scene_lights[i]->not_click_light();
					volumes->volume_interface->hide();
					selected_model = index - 1;
					scene_model->click_model();
					selecting_model = true;
					selecting_light = false;
					selecting_volume = false;
					transfer_funtion->hide = true;
				}
				else
				{
					scene_model->not_click_model();
					volumes->volume_interface->hide();
					scene_lights[index - 2]->click_light();
					selecting_model = false;
					selected_model = -1;
					selected_light = index - 2;
					selecting_light = true;
					selecting_volume = false;
					transfer_funtion->hide = true;
				}
			}
			else {
				selecting_model = false;
				selected_model = -1;
				scene_model->not_click_model();
				for (int i = 0; i < num_of_lights; i++)
					scene_lights[i]->not_click_light();
				selecting_light = false;
			}
			if (transfer_funtion->click_transfer_f(x, y, g_width, g_height))
			{
				volumes->update_transfer_function(transfer_funtion->get_color_points());
				return;
			}
			if (volumes->click_volume(x, y, projection, view, scene_camera->position, false)) {
				selecting_volume = true;
				transfer_funtion->hide = false;
				volumes->volume_interface->show();
				scene_model->not_click_model();
				for (int i = 0; i < num_of_lights; i++)
					scene_lights[i]->not_click_light();
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
			transfer_funtion->disable_select();
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
	if (TwEventMousePosGLFW(x, y))
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

	if (transfer_funtion->poscursor_transfer_f(x, y, g_width, g_height))
	{
		volumes->update_transfer_function(transfer_funtion->get_color_points());
		return;
	}
	/*if (volumes->poscursorVolume(x, y))
		return;*/
}

void char_input(GLFWwindow* window, unsigned int scan_char)
{
	if (TwEventCharGLFW(scan_char, GLFW_PRESS))
		return;
}

void drop_path(GLFWwindow* window, int count, const char** paths)
{
	volumes->drop_path(count, paths);
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
		scene_camera->process_keyboard(DONW, delta_time);
}

bool init_glfw()
{
	g_width = 1200;
	g_height = 680;

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

		glsl_g_buffer.loadShader("Shaders/gBuffer.vert", CGLSLProgram::VERTEX);
		glsl_g_buffer.loadShader("Shaders/gBuffer.frag", CGLSLProgram::FRAGMENT);
		glsl_g_buffer.loadShader("Shaders/gBuffer.geom", CGLSLProgram::GEOMETRY);
		glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.vert", CGLSLProgram::VERTEX);
		glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.frag", CGLSLProgram::FRAGMENT);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.vert", CGLSLProgram::VERTEX);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.frag", CGLSLProgram::FRAGMENT);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.geom", CGLSLProgram::GEOMETRY);
		/*glsl_mipmaps.loadShader("Shaders/mipmap.vert", CGLSLProgram::VERTEX);
		glsl_mipmaps.loadShader("Shaders/mipmap.frag", CGLSLProgram::FRAGMENT);
		glsl_mipmaps.loadShader("Shaders/mipmap.geom", CGLSLProgram::GEOMETRY);*/
		glsl_blending.loadShader("Shaders/blending.vert", CGLSLProgram::VERTEX);
		glsl_blending.loadShader("Shaders/blending.frag", CGLSLProgram::FRAGMENT);
		glsl_test.loadShader("Shaders/test.vert", CGLSLProgram::VERTEX);
		glsl_test.loadShader("Shaders/test.frag", CGLSLProgram::FRAGMENT);

		glsl_g_buffer.create_link();
		glsl_g_buffer_plane.create_link();
		glsl_scattered_map.create_link();
		//glsl_mipmaps.create_link();
		glsl_blending.create_link();
		glsl_test.create_link();

		glsl_g_buffer.enable();
		glsl_g_buffer.addAttribute("position");
		glsl_g_buffer.addAttribute("normal");

		glsl_g_buffer.addUniform("model_matrix");
		glsl_g_buffer.addUniform("vp_light");
		glsl_g_buffer.addUniform("num_of_lights");
		glsl_g_buffer.addUniform("num_of_buffer");
		glsl_g_buffer.disable();

		glsl_g_buffer_plane.enable();
		glsl_g_buffer_plane.addAttribute("position");
		glsl_g_buffer_plane.addAttribute("tex_coords");

		glsl_g_buffer_plane.addUniform("model_matrix");
		glsl_g_buffer_plane.addUniform("position_tex");
		glsl_g_buffer_plane.disable();

		glsl_scattered_map.enable();
		glsl_scattered_map.addAttribute("position");
		glsl_scattered_map.addAttribute("normal");

		glsl_scattered_map.addUniform("n_cameras");
		glsl_scattered_map.addUniform("cameras_matrix");
		glsl_scattered_map.addUniform("vp_light");
		glsl_scattered_map.addUniform("model_matrix");

		glsl_scattered_map.addUniform("asymmetry_param_g");
		glsl_scattered_map.addUniform("light_pos");
		glsl_scattered_map.addUniform("light_diff");
		glsl_scattered_map.addUniform("n_samples");
		glsl_scattered_map.addUniform("num_of_lights");
		glsl_scattered_map.addUniform("samples");
		glsl_scattered_map.addUniform("g_position");
		glsl_scattered_map.addUniform("g_normal");
		glsl_scattered_map.addUniform("g_depth");
		glsl_scattered_map.addUniform("radius");
		glsl_scattered_map.addUniform("refractive_index");
		glsl_scattered_map.addUniform("diffuse_reflectance");

		// Valores pre-calculados
		glsl_scattered_map.addUniform("attenuation_coeff");
		glsl_scattered_map.addUniform("D");
		glsl_scattered_map.addUniform("effective_transport_coeff");
		glsl_scattered_map.addUniform("c_phi_1");
		glsl_scattered_map.addUniform("c_phi_2");
		glsl_scattered_map.addUniform("c_e");
		glsl_scattered_map.addUniform("A");
		glsl_scattered_map.addUniform("de");
		glsl_scattered_map.addUniform("zr");
		glsl_scattered_map.disable();

		/*glsl_mipmaps.enable();
			glsl_mipmaps.addAttribute("position");
			glsl_mipmaps.addAttribute("normal");

			glsl_mipmaps.addUniform("n_cameras");
			glsl_mipmaps.addUniform("cameras_matrix");
		glsl_mipmaps.disable();*/

		glsl_blending.enable();
		glsl_blending.addAttribute("position");
		glsl_blending.addAttribute("normal");

		glsl_blending.addUniform("MVP");
		glsl_blending.addUniform("model_matrix");
		glsl_blending.addUniform("scattered_map");
		glsl_blending.addUniform("depth_map");
		glsl_blending.addUniform("camera_pos");
		glsl_blending.addUniform("light_pos");
		glsl_blending.addUniform("epsilon");
		glsl_blending.addUniform("refractive_index");
		glsl_blending.addUniform("n_cameras");
		glsl_blending.addUniform("cameras_matrix");
		glsl_blending.addUniform("cameras_dirs");
		glsl_blending.addUniform("gamma");
		glsl_blending.addUniform("current_frame");
		glsl_blending.addUniform("g_width");
		glsl_blending.addUniform("g_height");
		glsl_blending.disable();


		glsl_test.enable();
		glsl_test.addAttribute("vertex_coords");
		glsl_test.addAttribute("volume_coords");
		glsl_test.addUniform("MVP");
		glsl_test.addUniform("model_matrix");
		glsl_test.addUniform("axis");
		glsl_test.addUniform("start_texture");
		glsl_test.addUniform("position");
		glsl_test.addUniform("transfer_function_text");
		glsl_test.addUniform("volume_text");
		glsl_test.addUniform("previous_text");
		glsl_test.addUniform("iteration");
		glsl_test.addUniform("actual_texture");
		glsl_test.addUniform("light_pos");
		glsl_test.addUniform("normal");
		glsl_test.addUniform("volume_size");
		glsl_test.addUniform("vp_matrix");
		glsl_test.disable();

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
	light_buffer *g_buffer;
	material *potato, *marble, *skin, *milk, *cream, *none;

	num_of_lights = 1;
	num_of_ortho_cameras = 6;
	num_of_samples_per_frag = 3 * num_of_ortho_cameras;

	scene_model = new mesh();
	halton_generator = new halton();
	materials = new materials_set();
	light_buffers = new light_buffer(g_width, g_height, num_of_lights);
	transfer_funtion = new interface_function();
	volumes = new volume_render(g_width, g_height);
	volumes->update_transfer_function(transfer_funtion->get_color_points());
	transfer_funtion->hide = true;

	for (size_t i = 0; i < num_of_lights; i++)
	{
		scene_lights.push_back(new light());
	}
	//scene_lights[1]->translation = glm::vec3(3.0, 5.0f, 0.0f);
	//scene_lights[2]->translation = glm::vec3(0.0f, 0.0f, 5.0f);
	selected_light = 0;

	potato = new material(glm::vec3(0.68f, 0.70f, 0.55f), glm::vec3(0.0024f, 0.0090f, 0.12f), glm::vec3(0.77f, 0.62f, 0.21f), 1.3f);
	marble = new material(glm::vec3(2.19f, 2.62f, 3.00f), glm::vec3(0.0021f, 0.0041f, 0.0071f), glm::vec3(0.83f, 0.79f, 0.75f), 1.5f);
	skin = new material(glm::vec3(0.74f, 0.88f, 1.01f), glm::vec3(0.032f, 0.17f, 0.48f), glm::vec3(0.44f, 0.22f, 0.13f), 1.3f);
	milk = new material(glm::vec3(2.55f, 3.21f, 3.77f), glm::vec3(0.0011f, 0.0024f, 0.014f), glm::vec3(0.91f, 0.88f, 0.76f), 1.3f);
	cream = new material(glm::vec3(7.38f, 5.47f, 3.1f), glm::vec3(0.0002f, 0.0028f, 0.016f), glm::vec3(0.98f, 0.90f, 0.7f), 1.3f);
	none = new material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.3f);

	materials->materials.push_back(potato);
	materials->materials.push_back(marble);
	materials->materials.push_back(skin);
	materials->materials.push_back(milk);
	materials->materials.push_back(cream);
	materials->materials.push_back(none);

	scattered_maps = new scattered_map(g_width, g_height, num_of_ortho_cameras);
	scene_camera = new camera(glm::vec3(0.0f, 0.0f, 16.0f));
	scene_model->load("Models/obj/bunny.obj");
	const char** paths = new const char*[1];
	paths[0] = "Models\\raw\\bucky_32x32x32_8.raw";
	volumes->drop_path(1, paths);
	halton_generator->generate_orthographic_cameras(num_of_ortho_cameras);

	create_gbuffer();
	create_quad_light_volume();

	return true;
}

void display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projection_ortho, view_ortho, view_proj_ortho_light, view_proj_ortho_random, model_mat;
	glm::vec3 sigma_tr;

	view = scene_camera->get_view_matrix();
	projection = glm::perspective(scene_camera->zoom, (float)g_width / (float)g_height, 0.1f, 100.0f);
	projection_ortho = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 20.0f);

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, light_buffers->g_buffer[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glsl_g_buffer.enable();
		std::vector<glm::mat4> vp_light_set;

		for (size_t j = 0; j < num_of_lights; j++) {
			view_ortho = glm::lookAt(scene_lights[j]->translation, scene_model->center, glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_light = projection_ortho * view_ortho;
			vp_light_set.push_back(view_proj_ortho_light);
		}
		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, scene_model->translation);
		model_mat = model_mat * glm::toMat4(scene_model->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(scene_model->scale));

		glUniformMatrix4fv(glsl_g_buffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glsl_g_buffer.getLocation("vp_light"), num_of_lights, GL_FALSE, glm::value_ptr(vp_light_set[0]));
		glUniform1i(glsl_g_buffer.getLocation("num_of_lights"), num_of_lights);
		glUniform1i(glsl_g_buffer.getLocation("num_of_buffer"), i);

		glBindVertexArray(scene_model->vao);
		glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
		glBindVertexArray(0);
		glsl_g_buffer.disable();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	/*glsl_scattered_map.enable();
	if (scene_model->change_values || change_light)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, scattered_maps->buffer);
		glStencilFunc(GL_ALWAYS, 1, -1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		materials->materials[scene_model->current_material]->precalculate_values(scene_model->asymmetry_param_g);
		sigma_tr = materials->materials[scene_model->current_material]->effective_transport_coeff;
		halton_generator->generate_samples(min(sigma_tr.x, sigma_tr.y, sigma_tr.z) / scene_model->q, scene_model->radius, num_of_samples_per_frag);

		std::vector<glm::mat4> view_proj_ortho_randoms, vp_light_set;

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, scene_model->translation);
		model_mat = model_mat * glm::toMat4(scene_model->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(scene_model->scale));

		for (size_t j = 0; j < num_of_ortho_cameras; j++)
		{
			view_ortho = glm::lookAt(halton_generator->camera_positions[j], scene_model->center, glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_random = projection_ortho * view_ortho;
			view_proj_ortho_randoms.push_back(view_proj_ortho_random);
		}

		std::vector<glm::vec3> light_pos_set;

		for (size_t j = 0; j < num_of_lights; j++) {
			light_pos_set.push_back(scene_lights[j]->translation);
			view_ortho = glm::lookAt(scene_lights[j]->translation, scene_model->center, glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_light = projection_ortho * view_ortho;
			vp_light_set.push_back(view_proj_ortho_light);
		}

		glUniformMatrix4fv(glsl_scattered_map.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniform1i(glsl_scattered_map.getLocation("n_cameras"), num_of_ortho_cameras);
		glUniformMatrix4fv(glsl_scattered_map.getLocation("cameras_matrix"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(view_proj_ortho_randoms[0]));
		glUniformMatrix4fv(glsl_scattered_map.getLocation("vp_light"), num_of_lights, GL_FALSE, glm::value_ptr(vp_light_set[0]));
		glUniform1i(glsl_scattered_map.getLocation("g_position"), 0);
		glUniform1i(glsl_scattered_map.getLocation("g_normal"), 1);
		glUniform1i(glsl_scattered_map.getLocation("g_depth"), 2);
		glUniform1f(glsl_scattered_map.getLocation("radius"), scene_model->radius);
		glUniform1i(glsl_scattered_map.getLocation("n_samples"), num_of_samples_per_frag);
		glUniform1i(glsl_scattered_map.getLocation("num_of_lights"), num_of_lights);
		glUniform2fv(glsl_scattered_map.getLocation("samples"), num_of_samples_per_frag, glm::value_ptr(halton_generator->samples[0]));
		glUniform1f(glsl_scattered_map.getLocation("asymmetry_param_g"), scene_model->asymmetry_param_g);
		glUniform1f(glsl_scattered_map.getLocation("refractive_index"), scene_model->refractive_index);
		glUniform3fv(glsl_scattered_map.getLocation("diffuse_reflectance"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->diffuse_reflectance));
		glUniform3fv(glsl_scattered_map.getLocation("light_pos"), num_of_lights, glm::value_ptr(light_pos_set[0]));
		glUniform4f(glsl_scattered_map.getLocation("light_diff"), 1.0f, 1.0f, 1.0f, 1.0f);

		// Valores pre-calculados
		glUniform3fv(glsl_scattered_map.getLocation("attenuation_coeff"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->attenuation_coeff));
		glUniform3fv(glsl_scattered_map.getLocation("D"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->D));
		glUniform3fv(glsl_scattered_map.getLocation("effective_transport_coeff"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->effective_transport_coeff));
		glUniform1f(glsl_scattered_map.getLocation("c_phi_1"), materials->materials[scene_model->current_material]->c_phi_1);
		glUniform1f(glsl_scattered_map.getLocation("c_phi_2"), materials->materials[scene_model->current_material]->c_phi_2);
		glUniform1f(glsl_scattered_map.getLocation("c_e"), materials->materials[scene_model->current_material]->c_e);
		glUniform1f(glsl_scattered_map.getLocation("A"), materials->materials[scene_model->current_material]->A);
		glUniform3fv(glsl_scattered_map.getLocation("de"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->de));
		glUniform3fv(glsl_scattered_map.getLocation("zr"), 1, glm::value_ptr(materials->materials[scene_model->current_material]->zr));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_position);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, light_buffers->g_depth);

		glBindVertexArray(scene_model->vao);
		glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		scene_model->change_values = false;
	}
	glsl_scattered_map.disable();*/

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/*glsl_blending.enable();
	glStencilFunc(GL_ALWAYS, 1, -1);
	glUniform3f(glsl_blending.getLocation("camera_pos"), scene_camera->position[0], scene_camera->position[1], scene_camera->position[2]);

	std::vector<glm::mat4> view_proj_ortho_randoms;
	std::vector<glm::vec3> cameras_dirs;

	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, scene_model->translation);
	model_mat = model_mat * glm::toMat4(scene_model->rotation);
	model_mat = glm::scale(model_mat, glm::vec3(scene_model->scale));

	for (size_t j = 0; j < num_of_ortho_cameras; j++)
	{
		view_ortho = glm::lookAt(halton_generator->camera_positions[j], scene_model->center, glm::vec3(0.0f, 1.0f, 0.0f));
		view_proj_ortho_random = projection_ortho * view_ortho;
		view_proj_ortho_randoms.push_back(view_proj_ortho_random);
		cameras_dirs.push_back(halton_generator->camera_positions[j]);
	}

	glUniformMatrix4fv(glsl_blending.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	glUniformMatrix4fv(glsl_blending.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model_mat));
	glUniform1i(glsl_blending.getLocation("scattered_map"), 0);
	glUniform1i(glsl_blending.getLocation("depth_map"), 1);
	glUniform1f(glsl_blending.getLocation("epsilon"), scene_model->epsilon);
	glUniform1f(glsl_blending.getLocation("refractive_index"), scene_model->refractive_index);
	glUniform1i(glsl_blending.getLocation("n_cameras"), num_of_ortho_cameras);
	glUniformMatrix4fv(glsl_blending.getLocation("cameras_matrix"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(view_proj_ortho_randoms[0]));
	glUniformMatrix4fv(glsl_blending.getLocation("cameras_dirs"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(cameras_dirs[0]));
	glUniform1f(glsl_blending.getLocation("gamma"), scene_model->gamma);
	glUniform1i(glsl_blending.getLocation("current_frame"), 1);
	glUniform1i(glsl_blending.getLocation("g_width"), g_width);
	glUniform1i(glsl_blending.getLocation("g_height"), g_height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, scattered_maps->array_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, scattered_maps->depth_texture);

	glBindVertexArray(scene_model->vao);
	glDrawArrays(GL_TRIANGLES, 0, scene_model->vertices.size());
	glBindVertexArray(0);
	glsl_blending.disable();*/

	for (int l = 0; l < num_of_lights; l++) {
		glStencilFunc(GL_ALWAYS, 2 + l, -1);
		scene_lights[l]->display(projection * view);
	}

	glDisable(GL_STENCIL_TEST);


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	int actual_texture;
	glm::vec4 position_sign, dir_max;
	glm::vec3 ray_step, position;
	float distance, lenght_in_out, step_size, texture_step, start_texture;
	GLvoid *img;

	actual_texture = 1;
	model = glm::translate(glm::mat4(1.0f), volumes->volumes[0]->translation) * glm::mat4_cast(volumes->volumes[0]->rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(volumes->volumes[0]->escalation));
	dir_max = volumes->calculate_dir_max(scene_lights[0]->translation, model);
	step_size = volumes->volumes[0]->step_light_volume;
	position_sign = volumes->get_position(volumes->volumes[0]->current_index);
	ray_step = (glm::vec3(dir_max.x, dir_max.y, dir_max.z) * step_size) * position_sign.w;
	texture_step = (step_size) * position_sign.w;
	position = glm::vec3(position_sign.x, position_sign.y, position_sign.z);

	if (position_sign.w == -1.0f)
		start_texture = 1.0f;
	else
		start_texture = 0.0f;

	glsl_test.enable();
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	lenght_in_out = glm::length(glm::vec3(dir_max.x, dir_max.y, dir_max.z));
	for (float i = 0.0f; i < step_size; i += step_size)
	{
		if (actual_texture == 1)
			actual_texture = 0;
		else
			actual_texture = 1;
		glUniformMatrix4fv(glsl_test.getLocation("MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * model));
		glUniformMatrix4fv(glsl_test.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glsl_test.getLocation("axis"), dir_max.w);
		glUniform1f(glsl_test.getLocation("start_texture"), start_texture);
		glUniform3fv(glsl_test.getLocation("position"), 1, &position[0]);
		glUniform1f(glsl_test.getLocation("iteration"), i);
		glUniform1i(glsl_test.getLocation("actual_texture"), actual_texture);
		glUniform3fv(glsl_test.getLocation("light_pos"), 1, glm::value_ptr(scene_lights[0]->translation));
		glUniform3fv(glsl_test.getLocation("normal"), 1, glm::value_ptr(-glm::vec3(dir_max.x, dir_max.y, dir_max.z)));
		glUniform3iv(glsl_test.getLocation("volume_size"), 1, glm::value_ptr(glm::ivec3(volumes->volumes[0]->width, volumes->volumes[0]->height, volumes->volumes[0]->depth)));
		glUniformMatrix4fv(glsl_test.getLocation("vp_matrix"), 1, GL_FALSE, glm::value_ptr(projection * view));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, volumes->transfer_function_text);
		glUniform1i(glsl_test.getLocation("transfer_function_text"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, volumes->volumes[0]->volume_text);
		glUniform1i(glsl_test.getLocation("volume_text"), 1);
		if (i > 0.0f)
		{
			glActiveTexture(GL_TEXTURE2);
			if (actual_texture == 0)
			{
				//glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA16F, GL_FLOAT, img);
				glBindTexture(GL_TEXTURE_2D, g_out_prev);
			}
			else
				glBindTexture(GL_TEXTURE_2D, g_out);
			glUniform1i(glsl_test.getLocation("previous_text"), 2);
		}
		//glBindImageTexture(4, vol_ilum, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindVertexArray(texture_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		position += ray_step;
		start_texture += texture_step;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glsl_test.disable();

	glDisable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);

	glsl_g_buffer_plane.enable();
	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, glm::vec3(0.7, -0.7, -1.0));
	model_mat = glm::scale(model_mat, glm::vec3(0.3f));
	glUniformMatrix4fv(glsl_g_buffer_plane.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_out);
	render_quad();
	glsl_g_buffer_plane.disable();

	volumes->display(projection * view, scene_camera->position, scene_lights[0]);
	transfer_funtion->display();
}

void destroy()
{
	glfwDestroyWindow(g_window);
	glfwTerminate();
	transfer_funtion->~interface_function();
}

int main()
{
	if (!init_glfw() || !init_glew() || !init_ant_tweak_bar() || !init_scene())
		return EXIT_FAILURE;

	glEnable(GL_DEPTH_TEST);
	reshape(g_window, g_width, g_height);

	while (!glfwWindowShouldClose(g_window))
	{
		current_frame = float(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		glfwPollEvents();
		movement();
		transfer_funtion->update_coords();
		display();
		TwDraw();
		change_light = scene_lights[selected_light]->update_interface();
		scene_model->update_interface();
		volumes->update_interface();
		glfwSwapBuffers(g_window);
	}

	destroy();

	return EXIT_SUCCESS;
}