#include "InterfaceLight.h"

interfaceLight * interfaceLight::userInterface = NULL;

interfaceLight * interfaceLight::instance()
{
	if (!userInterface)
		userInterface = new interfaceLight();

	return userInterface;
}

interfaceLight::interfaceLight()
{
	lightInterface = TwNewBar("Iluminaci�n");

	TwDefine("Iluminaci�n refresh = '0.0001f'");
	TwDefine("Iluminaci�n resizable = false");
	TwDefine("Iluminaci�n fontresizable = false");
	TwDefine("Iluminaci�n movable = false");
	TwDefine("Iluminaci�n visible = false");
	TwDefine("Iluminaci�n position = '20 20'");
	TwDefine("Iluminaci�n valueswidth = 100 ");
	TwDefine("Iluminaci�n color = '42 46 148' alpha = 85");
	TwDefine("Iluminaci�n size = '300 400'");

	TwAddVarRW(lightInterface, "onOff", TW_TYPE_BOOLCPP, &on, "label = 'Encendida'");
	TwAddSeparator(lightInterface, NULL, "");
	TwAddVarRW(lightInterface, "Posici�n X", TW_TYPE_FLOAT, &translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(lightInterface, "Posici�n Y", TW_TYPE_FLOAT, &translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(lightInterface, "Posici�n Z", TW_TYPE_FLOAT, &translation[2], "group = 'Transformaciones' step = 0.01");

	TwAddSeparator(lightInterface, NULL, "");
	TwAddVarRW(lightInterface, "c1M", TW_TYPE_COLOR3F, &ambient_comp, "group='Componentes' label = 'Ambiental'");
	TwAddVarRW(lightInterface, "c2M", TW_TYPE_COLOR3F, &diffuse_comp, "group='Componentes' label = 'Difuso'");
	TwAddVarRW(lightInterface, "c3M", TW_TYPE_COLOR3F, &specular_comp, "group='Componentes' label = 'Especular'");
}

interfaceLight::~interfaceLight()
{
}

void interfaceLight::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interfaceLight::show()
{
	TwDefine("Iluminaci�n visible = true");
}

void interfaceLight::hide()
{
	TwDefine("Iluminaci�n visible = false");
}