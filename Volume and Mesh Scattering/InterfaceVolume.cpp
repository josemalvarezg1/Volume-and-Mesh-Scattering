#include "InterfaceVolume.h"

interface_volume * interface_volume::user_interface = NULL;

interface_volume * interface_volume::instance()
{
	if (!user_interface)
		user_interface = new interface_volume();

	return user_interface;
}

interface_volume::interface_volume()
{
	this->volume_interface = TwNewBar("Volumen");
	TwDefine("Volumen refresh = '0.0001f'");
	TwDefine("Volumen resizable = false");
	TwDefine("Volumen fontresizable = false");
	TwDefine("Volumen movable = false");
	TwDefine("Volumen visible = false");
	TwDefine("Volumen position = '20 20'");
	TwDefine("Volumen valueswidth = 100 ");
	TwDefine("Volumen color = '42 46 148' alpha = 85");
	TwDefine("Volumen size = '300 250'");

	TwAddVarRW(this->volume_interface, "Escalar", TW_TYPE_FLOAT, &this->scale, "group='Transformaciones' min=0.01 step=0.01");
	TwAddVarRW(this->volume_interface, "Posición X", TW_TYPE_FLOAT, &this->translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->volume_interface, "Posición Y", TW_TYPE_FLOAT, &this->translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->volume_interface, "Posición Z", TW_TYPE_FLOAT, &this->translation[2], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->volume_interface, "Rotación", TW_TYPE_QUAT4F, &this->rotation, "group='Transformaciones' opened=true");
}

interface_volume::~interface_volume()
{
}

void interface_volume::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interface_volume::show()
{
	TwDefine("Volumen visible = true");
}

void interface_volume::hide()
{
	TwDefine("Volumen visible = false");
}