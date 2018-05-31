#include "InterfaceMenu.h"

interface_menu * interface_menu::user_interface = NULL;

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
	TwDefine("Menú size = '300 100'");

	TwAddVarRW(this->menu_interface, "num_of_cam", TW_TYPE_INT32, &this->num_of_cameras, "label = 'Número de cámaras' min=1");
	TwAddSeparator(this->menu_interface, NULL, "");
	TwAddVarRW(this->menu_interface, "actual_cam", TW_TYPE_INT32, &this->camera_selected, "label = 'Cámara actual' min=0");
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

void interface_menu::show()
{
	TwDefine("Menú visible = true");
}

void interface_menu::hide()
{
	TwDefine("Menú visible = false");
}