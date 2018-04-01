#include "InterfaceLight.h"

interface_light * interface_light::user_interface = NULL;

interface_light * interface_light::instance()
{
	if (!user_interface)
		user_interface = new interface_light();

	return user_interface;
}

interface_light::interface_light()
{
	this->light_interface = TwNewBar("Iluminaci�n");

	TwDefine("Iluminaci�n refresh = '0.0001f'");
	TwDefine("Iluminaci�n resizable = false");
	TwDefine("Iluminaci�n fontresizable = false");
	TwDefine("Iluminaci�n movable = false");
	TwDefine("Iluminaci�n visible = false");
	TwDefine("Iluminaci�n position = '20 20'");
	TwDefine("Iluminaci�n valueswidth = 100 ");
	TwDefine("Iluminaci�n color = '42 46 148' alpha = 85");
	TwDefine("Iluminaci�n size = '300 400'");

	TwAddVarRW(this->light_interface, "onOff", TW_TYPE_BOOLCPP, &this->on, "label = 'Encendida'");
	TwAddSeparator(this->light_interface, NULL, "");
	TwAddVarRW(this->light_interface, "Posici�n X", TW_TYPE_FLOAT, &this->translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->light_interface, "Posici�n Y", TW_TYPE_FLOAT, &this->translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->light_interface, "Posici�n Z", TW_TYPE_FLOAT, &this->translation[2], "group = 'Transformaciones' step = 0.01");

	TwAddSeparator(this->light_interface, NULL, "");
	TwAddVarRW(this->light_interface, "c1M", TW_TYPE_COLOR3F, &this->ambient_comp, "group='Componentes' label = 'Ambiental'");
	TwAddVarRW(this->light_interface, "c2M", TW_TYPE_COLOR3F, &this->diffuse_comp, "group='Componentes' label = 'Difuso'");
	TwAddVarRW(this->light_interface, "c3M", TW_TYPE_COLOR3F, &this->specular_comp, "group='Componentes' label = 'Especular'");
}

interface_light::~interface_light()
{
}

void interface_light::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interface_light::show()
{
	TwDefine("Iluminaci�n visible = true");
}

void interface_light::hide()
{
	TwDefine("Iluminaci�n visible = false");
}