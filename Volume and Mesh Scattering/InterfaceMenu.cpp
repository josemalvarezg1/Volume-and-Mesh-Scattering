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
	this->menu_interface = TwNewBar("Men�");

	TwDefine("Men� refresh = '0.0001f'");
	TwDefine("Men� resizable = false");
	TwDefine("Men� fontresizable = false");
	TwDefine("Men� movable = false");
	TwDefine("Men� visible = true");
	TwDefine("Men� position = '850 20'");
	TwDefine("Men� valueswidth = 100 ");
	TwDefine("Men� color = '42 148 100' alpha = 85");
	TwDefine("Men� size = '300 100'");

	TwAddVarRW(this->menu_interface, "num_of_cam", TW_TYPE_INT32, &this->num_of_cameras, "label = 'N�mero de c�maras' min=1");
	TwAddSeparator(this->menu_interface, NULL, "");
	TwAddVarRW(this->menu_interface, "actual_cam", TW_TYPE_INT32, &this->camera_selected, "label = 'C�mara actual' min=0");
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
	TwDefine("Men� visible = true");
}

void interface_menu::hide()
{
	TwDefine("Men� visible = false");
}