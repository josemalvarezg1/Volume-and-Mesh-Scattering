#include "InterfaceLight.h"

extern int gWidth, gHeight;

interfaceLight * interfaceLight::userInterface = NULL;

interfaceLight * interfaceLight::instance()
{
	if (!userInterface)
		userInterface = new interfaceLight();

	return userInterface;
}

interfaceLight::interfaceLight()
{
	lightInterface = TwNewBar("Ilumination");

	TwDefine("Ilumination refresh = '0.0001f'");
	TwDefine("Ilumination resizable = false");
	TwDefine("Ilumination fontresizable = false");
	TwDefine("Ilumination movable = false");
	TwDefine("Ilumination visible = false");
	TwDefine("Ilumination position = '20 20'");
	TwDefine("Ilumination valueswidth = 100 ");
	TwDefine("Ilumination color = '42 46 148' alpha = 85");
	TwDefine("Ilumination size = '300 300'");

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

void interfaceLight::reshape()
{
	TwWindowSize(gWidth, gHeight);
}

void interfaceLight::show()
{
	TwDefine("Ilumination visible = true");
}

void interfaceLight::hide()
{
	TwDefine("Ilumination visible = false");
}