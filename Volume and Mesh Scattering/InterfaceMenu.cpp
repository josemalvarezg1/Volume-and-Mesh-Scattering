#include "InterfaceMenu.h"

interface_menu * interface_menu::user_interface = NULL;
extern bool scattering_model, scattering_volume, model_center, specular_flag;

interface_menu * interface_menu::instance()
{
	if (!user_interface)
		user_interface = new interface_menu();

	return user_interface;
}

interface_menu::interface_menu()
{
	this->menu_interface = TwNewBar("Menú");

	TwDefine("Menú refresh = '0.0001f'");
	TwDefine("Menú resizable = false");
	TwDefine("Menú fontresizable = false");
	TwDefine("Menú movable = false");
	TwDefine("Menú visible = true");
	TwDefine("Menú position = '850 20'");
	TwDefine("Menú valueswidth = 100 ");
	TwDefine("Menú color = '42 148 100' alpha = 85");
	TwDefine("Menú size = '300 200'");

	TwAddVarRW(this->menu_interface, "num_of_cam", TW_TYPE_INT32, &this->num_of_cameras, "group = 'Mallado' label = 'Número de cámaras' min=1");
	{
		TwEnumVal texture_type[4] = { { Scattered_Map, "Scattered Map" },{ GBuffer_Light_Position, "G-Buffer (Posición)" },{ GBuffer_Light_Normal, "G-Buffer (Normal)" },{ GBuffer_Light_Depth, "G-Buffer (Profundidad)" } };
		TwType light = TwDefineEnum("texture_type", texture_type, 4);
		TwAddVarRW(this->menu_interface, "Textura", light, &this->current_texture_type, "group='Mallado'");
	}
	TwAddVarRW(this->menu_interface, "actual_cam", TW_TYPE_INT32, &this->camera_selected, "group = 'Mallado' label = 'Cámara actual' min=0");
	TwAddVarCB(this->menu_interface, "center_model", TW_TYPE_BOOL32, set_model_center, get_model_center, NULL, " label='Centro del modelo' group='Mallado'");
	TwAddVarCB(this->menu_interface, "scattering_model", TW_TYPE_BOOL32, set_model_scattering, get_model_scattering, NULL, " label='Scattering' group='Mallado'");
	TwAddVarCB(this->menu_interface, "specular_flag", TW_TYPE_BOOL32, set_model_specular, get_model_specular, NULL, " label='Especular' group='Mallado'");
	TwAddSeparator(this->menu_interface, NULL, "");
	TwAddVarCB(this->menu_interface, "scattering_volume", TW_TYPE_BOOL32, set_volume_scattering, get_volume_scattering, NULL, " label='Scattering' group='Volumen'");
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

	window_position[0] = g_width - 320;
	window_position[1] = 20;
	TwSetParam(this->menu_interface, NULL, "position", TW_PARAM_INT32, 2, window_position);
}

void interface_menu::set_max_values(int num_of_cameras)
{
	TwSetParam(this->menu_interface, "actual_cam", "max", TW_PARAM_INT32, 1, &num_of_cameras);
}

void interface_menu::update_position()
{
	int window_position[2];

	window_position[0] = 20;
	window_position[1] = 440;
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

void TW_CALL set_model_center(const void *value, void *clientData) {
	model_center = *(const int *)value;
}

void TW_CALL get_model_center(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = model_center;
}

void TW_CALL set_model_specular(const void *value, void *clientData) {
	specular_flag = *(const int *)value;
}

void TW_CALL get_model_specular(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = specular_flag;
}