#include "Main.h"

GLFWwindow *g_window;
int g_width, g_height;
GLuint num_of_lights, num_of_ortho_cameras, num_of_samples_per_frag;
GLfloat delta_time = 0.0f, last_frame = 0.0f, current_frame;
GLdouble last_x = 600.0, last_y = 340.0;
bool keys[1024], keys_pressed[1024], selecting_model = false, selecting_light = false, first_mouse = true, activate_camera = false, change_light = false, selecting_volume = false;
scattered_map *scattered_maps;
halton *halton_generator;
glm::mat4 projection, view, model;

light *scene_light;
camera *scene_camera;
mesh_set *m_set;
light_buffers_set *light_buffers;
materials_set *materials;
interface_function *transfer_funtion;
volumeRender *volumes;

CGLSLProgram glsl_blinn, glsl_g_buffer, glsl_g_buffer_plane, glsl_scattered_map;
int selected_model = -1;
GLuint quad_vao, quad_vbo;

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

	volumes->resizeScreen(glm::vec2(g_width, g_height));
	scene_light->light_interface->reshape(g_width, g_height);
	m_set->model_interface->reshape(g_width, g_height);

	for (size_t i = 0; i < num_of_lights; i++)
		light_buffers->array_of_buffers[0]->update_g_buffer(g_width, g_height);

	for (size_t i = 0; i < m_set->mesh_models.size(); i++)
		m_set->mesh_models[i]->change_values = true;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
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
					m_set->not_click_model();
				if (selecting_light)
					scene_light->not_click_light();
				if (selecting_volume) {
					transfer_funtion->hide = true;
					volumes->volume_interface->hide();
				}
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (selecting_model)
					m_set->click_model(selected_model);
				if (selecting_light)
					scene_light->click_light();
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
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			GLuint index;
			glReadPixels(int(x), g_height - int(y), 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

			if (index > 0)
			{
				if (index <= m_set->mesh_models.size())
				{
					scene_light->not_click_light();
					volumes->volume_interface->hide();
					selected_model = index - 1;
					m_set->click_model(selected_model);
					selecting_model = true;
					selecting_light = false;
					selecting_volume = false;
					transfer_funtion->hide = true;
				}
				else
				{
					m_set->not_click_model();
					volumes->volume_interface->hide();
					scene_light->click_light();
					selecting_model = false;
					selected_model = -1;
					selecting_light = true;
					transfer_funtion->hide = true;
				}
			}
			else {
				selecting_model = false;
				selecting_volume = false;
				selected_model = -1;
				m_set->not_click_model();
				scene_light->not_click_light();
				selecting_light = false;
			}
			if (transfer_funtion->click_transfer_f(x, y, g_width, g_height))
			{
				volumes->UpdateTransferFunction(transfer_funtion->get_color_points());
				return;
			}
			if (volumes->clickVolume(x, y, projection, view, scene_camera->position, false)) {
				selecting_volume = true;
				transfer_funtion->hide = false;
				volumes->volume_interface->show();
				m_set->not_click_model();
				scene_light->not_click_light();
				return;
			}
		}
		else
		{
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
			{
				if (volumes->clickVolume(x, y, projection, view, scene_camera->position, true))
					return;
			}
		}
	} else if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			transfer_funtion->disable_select();
			volumes->disableSelect();
		}
		else
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				volumes->disableSelect();
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
		volumes->UpdateTransferFunction(transfer_funtion->get_color_points());
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
	volumes->dropPath(count, paths);
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

		glsl_blinn.loadShader("Shaders/program.vert", CGLSLProgram::VERTEX);
		glsl_blinn.loadShader("Shaders/program.frag", CGLSLProgram::FRAGMENT);
		glsl_g_buffer.loadShader("Shaders/gBuffer.vert", CGLSLProgram::VERTEX);
		glsl_g_buffer.loadShader("Shaders/gBuffer.frag", CGLSLProgram::FRAGMENT);
		glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.vert", CGLSLProgram::VERTEX);
		glsl_g_buffer_plane.loadShader("Shaders/gBufferPosition.frag", CGLSLProgram::FRAGMENT);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.vert", CGLSLProgram::VERTEX);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.frag", CGLSLProgram::FRAGMENT);
		glsl_scattered_map.loadShader("Shaders/scatteredMap.geom", CGLSLProgram::GEOMETRY);

		glsl_blinn.create_link();
		glsl_g_buffer.create_link();
		glsl_g_buffer_plane.create_link();
		glsl_scattered_map.create_link();

		glsl_blinn.enable();
			glsl_blinn.addAttribute("position");
			glsl_blinn.addAttribute("normal");

			glsl_blinn.addUniform("view_matrix");
			glsl_blinn.addUniform("projection_matrix");
			glsl_blinn.addUniform("model_matrix");
			glsl_blinn.addUniform("light_pos");
			glsl_blinn.addUniform("eye");
		glsl_blinn.disable();

		glsl_g_buffer.enable();
			glsl_g_buffer.addAttribute("position");
			glsl_g_buffer.addAttribute("normal");

			glsl_g_buffer.addUniform("light_matrix");
			glsl_g_buffer.addUniform("model_matrix");
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
			glsl_scattered_map.addUniform("samples");
			glsl_scattered_map.addUniform("g_position");
			glsl_scattered_map.addUniform("g_normal");
			glsl_scattered_map.addUniform("g_depth");
			glsl_scattered_map.addUniform("bias");
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
	mesh *scene_model;

	num_of_lights = 1;
	num_of_ortho_cameras = 1;
	num_of_samples_per_frag = 3 * num_of_ortho_cameras;

	scene_light = new light();
	scene_model = new mesh();
	m_set = new mesh_set();
	halton_generator = new halton();
	materials = new materials_set();
	light_buffers = new light_buffers_set();
	transfer_funtion = new interface_function();
	volumes = new volumeRender(g_width, g_height);
	volumes->UpdateTransferFunction(transfer_funtion->get_color_points());
	transfer_funtion->hide = true;

	for (size_t i = 0; i < num_of_lights; i++)
	{
		g_buffer = new light_buffer(g_width, g_height);
		light_buffers->array_of_buffers.push_back(g_buffer);
	}

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
	m_set->mesh_models.push_back(scene_model);
	halton_generator->generate_orthographic_cameras(num_of_ortho_cameras);

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

	for (size_t i = 0; i < num_of_lights; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, light_buffers->array_of_buffers[i]->g_buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glsl_g_buffer.enable();
		for (size_t j = 0; j < m_set->mesh_models.size(); j++)
		{
			view_ortho = glm::lookAt(scene_light->translation, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			view_proj_ortho_light = projection_ortho * view_ortho;

			model_mat = glm::mat4(1.0f);
			model_mat = glm::translate(model_mat, m_set->mesh_models[j]->translation);
			model_mat = model_mat * glm::toMat4(m_set->mesh_models[j]->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(m_set->mesh_models[j]->scale));

			glUniformMatrix4fv(glsl_g_buffer.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
			glUniformMatrix4fv(glsl_g_buffer.getLocation("vp_matrix"), 1, GL_FALSE, glm::value_ptr(view_proj_ortho_light));

			glBindVertexArray(m_set->mesh_models[j]->vao);
			glDrawArrays(GL_TRIANGLES, 0, m_set->mesh_models[j]->vertices.size());
			glBindVertexArray(0);
		}
		glsl_g_buffer.disable();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glsl_scattered_map.enable();
	for (size_t i = 0; i < m_set->mesh_models.size(); i++)
	{
		if (m_set->mesh_models[i]->change_values || change_light)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, scattered_maps->buffer);
			glStencilFunc(GL_ALWAYS, 1, -1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			materials->materials[m_set->mesh_models[i]->current_material]->precalculate_values(m_set->mesh_models[i]->asymmetry_param_g);
			sigma_tr = materials->materials[m_set->mesh_models[i]->current_material]->effective_transport_coeff;
			halton_generator->generate_samples(min(sigma_tr.x, sigma_tr.y, sigma_tr.z) / m_set->mesh_models[i]->q, m_set->mesh_models[i]->radius, num_of_samples_per_frag);

			std::vector<glm::mat4> view_proj_ortho_randoms;

			model_mat = glm::mat4(1.0f);
			model_mat = glm::translate(model_mat, m_set->mesh_models[i]->translation);
			model_mat = model_mat * glm::toMat4(m_set->mesh_models[i]->rotation);
			model_mat = glm::scale(model_mat, glm::vec3(m_set->mesh_models[i]->scale));

			for (size_t j = 0; j < num_of_ortho_cameras; j++)
			{
				view_ortho = glm::lookAt(halton_generator->camera_positions[j], m_set->mesh_models[i]->center, glm::vec3(0.0f, 1.0f, 0.0f));
				view_proj_ortho_random = projection_ortho * view_ortho;
				view_proj_ortho_randoms.push_back(view_proj_ortho_random);
			}

			glUniformMatrix4fv(glsl_scattered_map.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
			glUniform1i(glsl_scattered_map.getLocation("n_cameras"), num_of_ortho_cameras);
			glUniformMatrix4fv(glsl_scattered_map.getLocation("cameras_matrix"), num_of_ortho_cameras, GL_FALSE, glm::value_ptr(view_proj_ortho_randoms[0]));
			glUniformMatrix4fv(glsl_scattered_map.getLocation("vp_light"), 1, GL_FALSE, glm::value_ptr(view_proj_ortho_light));
			glUniform1i(glsl_scattered_map.getLocation("g_position"), 0);
			glUniform1i(glsl_scattered_map.getLocation("g_normal"), 1);
			glUniform1i(glsl_scattered_map.getLocation("g_depth"), 2);
			glUniform1f(glsl_scattered_map.getLocation("bias"), m_set->mesh_models[i]->bias);
			glUniform1f(glsl_scattered_map.getLocation("radius"), m_set->mesh_models[i]->radius);
			glUniform1i(glsl_scattered_map.getLocation("n_samples"), num_of_samples_per_frag);
			glUniform2fv(glsl_scattered_map.getLocation("samples"), num_of_samples_per_frag, glm::value_ptr(halton_generator->samples[0]));
			glUniform1f(glsl_scattered_map.getLocation("asymmetry_param_g"), m_set->mesh_models[i]->asymmetry_param_g);
			glUniform1f(glsl_scattered_map.getLocation("refractive_index"), m_set->mesh_models[i]->refractive_index);
			glUniform3fv(glsl_scattered_map.getLocation("diffuse_reflectance"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->diffuse_reflectance));

			glUniform3f(glsl_scattered_map.getLocation("light_pos"), scene_light->translation.x, scene_light->translation.y, scene_light->translation.z);
			glUniform4f(glsl_scattered_map.getLocation("light_diff"), 1.0f, 1.0f, 1.0f, 1.0f);

			// Valores pre-calculados
			glUniform3fv(glsl_scattered_map.getLocation("attenuation_coeff"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->attenuation_coeff));
			glUniform3fv(glsl_scattered_map.getLocation("D"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->D));
			glUniform3fv(glsl_scattered_map.getLocation("effective_transport_coeff"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->effective_transport_coeff));
			glUniform1f(glsl_scattered_map.getLocation("c_phi_1"), materials->materials[m_set->mesh_models[i]->current_material]->c_phi_1);
			glUniform1f(glsl_scattered_map.getLocation("c_phi_2"), materials->materials[m_set->mesh_models[i]->current_material]->c_phi_2);
			glUniform1f(glsl_scattered_map.getLocation("c_e"), materials->materials[m_set->mesh_models[i]->current_material]->c_e);
			glUniform1f(glsl_scattered_map.getLocation("A"), materials->materials[m_set->mesh_models[i]->current_material]->A);
			glUniform3fv(glsl_scattered_map.getLocation("de"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->de));
			glUniform3fv(glsl_scattered_map.getLocation("zr"), 1, glm::value_ptr(materials->materials[m_set->mesh_models[i]->current_material]->zr));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[0]->g_position);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[0]->g_normal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, light_buffers->array_of_buffers[0]->g_depth);

			glBindVertexArray(m_set->mesh_models[i]->vao);
			glDrawArrays(GL_TRIANGLES, 0, m_set->mesh_models[i]->vertices.size());
			glBindVertexArray(0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			m_set->mesh_models[i]->change_values = false;
		}
	}

	glsl_scattered_map.disable();

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glsl_blinn.enable();
	for (size_t i = 0; i < m_set->mesh_models.size(); i++)
	{
		glStencilFunc(GL_ALWAYS, i + 1, -1);
		glUniform3f(glsl_blinn.getLocation("eye"), scene_camera->position[0], scene_camera->position[1], scene_camera->position[2]);
		glUniform3f(glsl_blinn.getLocation("light_pos"), scene_light->translation.x, scene_light->translation.y, scene_light->translation.z);

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, m_set->mesh_models[i]->translation);
		model_mat = model_mat * glm::toMat4(m_set->mesh_models[i]->rotation);
		model_mat = glm::scale(model_mat, glm::vec3(m_set->mesh_models[i]->scale));

		glUniformMatrix4fv(glsl_blinn.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(glsl_blinn.getLocation("view_matrix"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glsl_blinn.getLocation("projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(m_set->mesh_models[i]->vao);
		glDrawArrays(GL_TRIANGLES, 0, m_set->mesh_models[i]->vertices.size());
		glBindVertexArray(0);
	}
	glsl_blinn.disable();

	glStencilFunc(GL_ALWAYS, m_set->mesh_models.size() + 1, -1);
	scene_light->display(projection * view);

	glDisable(GL_STENCIL_TEST);

	glsl_g_buffer_plane.enable();
	model_mat = glm::mat4(1.0f);
	model_mat = glm::translate(model_mat, glm::vec3(0.7, -0.7, -1.0));
	model_mat = glm::scale(model_mat, glm::vec3(0.3f));
	glUniformMatrix4fv(glsl_g_buffer_plane.getLocation("model_matrix"), 1, GL_FALSE, glm::value_ptr(model_mat));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, scattered_maps->array_texture);
	render_quad();
	glsl_g_buffer_plane.disable();

	glDisable(GL_DEPTH_TEST);

	volumes->display(projection * view, scene_camera->position, scene_light->translation, scene_light->on, scene_light->ambient_comp, scene_light->diffuse_comp, scene_light->specular_comp);
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
		change_light = scene_light->update_interface();
		m_set->update_interface(selected_model);
		volumes->update_interface();
		glfwSwapBuffers(g_window);
	}

	destroy();

	return EXIT_SUCCESS;
}