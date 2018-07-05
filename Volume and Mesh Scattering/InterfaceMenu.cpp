#include "InterfaceMenu.h"

interface_menu * interface_menu::user_interface = NULL;
extern bool scattering_model, scattering_volume, model_center, specular_flag, gradient_volume;

interface_menu * interface_menu::instance()
{
	if (!user_interface)
		user_interface = new interface_menu();

	return user_interface;
}

interface_menu::interface_menu()
{
	this->menu_interface = TwNewBar("Menú");
	this->shininess = 128.0f;
	this->current_model = Bunny;
	this->current_volume = Bucky;

	TwDefine("Menú refresh = '0.0001f'");
	TwDefine("Menú resizable = false");
	TwDefine("Menú fontresizable = false");
	TwDefine("Menú movable = false");
	TwDefine("Menú visible = true");
	TwDefine("Menú position = '850 20'");
	TwDefine("Menú valueswidth = 100 ");
	TwDefine("Menú color = '42 148 100' alpha = 85");
	TwDefine("Menú size = '300 250'");

	{
		TwEnumVal models[5] = { { Bunny, "Bunny" },{ Hebe, "Hebe" },{ Buddha, "Buddha" },{ Dragon, "Dragon" },{ Esfera, "Esfera" } };
		TwType model = TwDefineEnum("model", models, 5);
		TwAddVarRW(this->menu_interface, "Modelo", model, &this->current_model, "group='Mallado'");
	}
	TwAddVarRW(this->menu_interface, "num_of_cam", TW_TYPE_INT32, &this->num_of_cameras, "group = 'Mallado' label = 'Número de cámaras' min=1");
	{
		TwEnumVal texture_type[4] = { { Scattered_Map, "Scattered Map" },{ GBuffer_Light_Position, "G-Buffer (Posición)" },{ GBuffer_Light_Normal, "G-Buffer (Normal)" },{ GBuffer_Light_Depth, "G-Buffer (Profundidad)" } };
		TwType type = TwDefineEnum("texture_type", texture_type, 4);
		TwAddVarRW(this->menu_interface, "Textura", type, &this->current_texture_type, "group='Mallado'");
	}
	TwAddVarRW(this->menu_interface, "actual_cam", TW_TYPE_INT32, &this->camera_selected, "group = 'Mallado' label = 'Cámara actual' min=0");
	TwAddVarCB(this->menu_interface, "center_model", TW_TYPE_BOOL32, set_model_center, get_model_center, NULL, " label='Centro del modelo' group='Mallado'");
	TwAddVarCB(this->menu_interface, "scattering_model", TW_TYPE_BOOL32, set_model_scattering, get_model_scattering, NULL, " label='Scattering' group='Mallado'");
	TwAddVarCB(this->menu_interface, "specular_flag", TW_TYPE_BOOL32, set_model_specular, get_model_specular, NULL, " label='Especular' group='Mallado'");
	TwAddVarRW(this->menu_interface, "shininess", TW_TYPE_FLOAT, &this->shininess, "group = 'Mallado' label = 'Shininess' min=1.0 step = 0.01 visible=false");

	TwAddSeparator(this->menu_interface, NULL, "");
	{
		TwEnumVal models[3] = { { Bucky, "Futboleno" },{ Bonsai, "Bonsai" },{ Engine, "Motor" } };
		TwType model = TwDefineEnum("volume", models, 3);
		TwAddVarRW(this->menu_interface, "volume_v", model, &this->current_volume, "group='Volumen' label='Volumen'");
	}
	TwAddVarCB(this->menu_interface, "scattering_volume", TW_TYPE_BOOL32, set_volume_scattering, get_volume_scattering, NULL, " label='Scattering' group='Volumen'");
	TwAddVarCB(this->menu_interface, "gradient_volume", TW_TYPE_BOOL32, set_volume_gradient, get_volume_gradient, NULL, " label='Gradientes' group='Volumen'");
}

interface_menu::~interface_menu()
{
}

void interface_menu::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interface_menu::update_width(int g_width)
{
	int window_position[2];

	if (g_width > 320)
	{
		window_position[0] = g_width - 320;
		window_position[1] = 20;
		TwSetParam(this->menu_interface, NULL, "position", TW_PARAM_INT32, 2, window_position);
	}

}

void interface_menu::set_max_values(int num_of_cameras)
{
	TwSetParam(this->menu_interface, "actual_cam", "max", TW_PARAM_INT32, 1, &num_of_cameras);
}

void interface_menu::update_position()
{
	int window_position[2];

	window_position[0] = 20;
	window_position[1] = 290;
	TwSetParam(this->menu_interface, NULL, "position", TW_PARAM_INT32, 2, window_position);
}

void interface_menu::show()
{
	TwDefine("Menú visible = true");
}

void interface_menu::hide()
{
	TwDefine("Menú visible = false");
}

void TW_CALL set_model_scattering(const void *value, void *clientData) {
	scattering_model = *(const int *)value;
	if (!scattering_model) {
		TwDefine("Menú/shininess visible = true");
		TwDefine("Menú/specular_flag visible = false");
	}
	else {
		TwDefine("Menú/shininess visible = false");
		TwDefine("Menú/specular_flag visible = true");
	}
}

void TW_CALL get_model_scattering(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = scattering_model;
}

void TW_CALL set_volume_scattering(const void *value, void *clientData) {
	scattering_volume = *(const int *)value;
}

void TW_CALL get_volume_scattering(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = scattering_volume;
}

void TW_CALL set_volume_gradient(const void *value, void *clientData) {
	gradient_volume = *(const int *)value;
}

void TW_CALL get_volume_gradient(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = gradient_volume;
}


void TW_CALL set_model_center(const void *value, void *clientData) {
	model_center = *(const int *)value;
}

void TW_CALL get_model_center(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = model_center;
}

void TW_CALL set_model_specular(const void *value, void *clientData) {
	specular_flag = *(const int *)value;
	if (specular_flag)
		TwDefine("Menú/shininess visible = true");
	else
		TwDefine("Menú/shininess visible = false");
}

void TW_CALL get_model_specular(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = specular_flag;
}