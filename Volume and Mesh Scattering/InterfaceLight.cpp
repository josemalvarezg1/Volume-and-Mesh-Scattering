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
	lightInterface = TwNewBar("Iluminación");

	TwDefine("Iluminación refresh = '0.0001f'");
	TwDefine("Iluminación resizable = false");
	TwDefine("Iluminación fontresizable = false");
	TwDefine("Iluminación movable = false");
	TwDefine("Iluminación visible = false");
	TwDefine("Iluminación position = '20 20'");
	TwDefine("Iluminación valueswidth = 100 ");
	TwDefine("Iluminación color = '42 46 148' alpha = 85");
	TwDefine("Iluminación size = '300 400'");

	TwAddVarRW(lightInterface, "onOff", TW_TYPE_BOOLCPP, &on, "label = 'Encendida'");
	TwAddSeparator(lightInterface, NULL, "");
	TwAddVarRW(lightInterface, "Posición X", TW_TYPE_FLOAT, &translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(lightInterface, "Posición Y", TW_TYPE_FLOAT, &translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(lightInterface, "Posición Z", TW_TYPE_FLOAT, &translation[2], "group = 'Transformaciones' step = 0.01");

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
	TwDefine("Iluminación visible = true");
}

void interfaceLight::hide()
{
	TwDefine("Iluminación visible = false");
}