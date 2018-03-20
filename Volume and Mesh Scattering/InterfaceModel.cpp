#include "InterfaceModel.h"

interfaceModel * interfaceModel::userInterface = NULL;

interfaceModel * interfaceModel::instance()
{
	if (!userInterface)
		userInterface = new interfaceModel();

	return userInterface;
}

interfaceModel::interfaceModel()
{
	modelInterface = TwNewBar("Modelo");
	TwDefine("Modelo refresh = '0.0001f'");
	TwDefine("Modelo resizable = false");
	TwDefine("Modelo fontresizable = false");
	TwDefine("Modelo movable = false");
	TwDefine("Modelo visible = false");
	TwDefine("Modelo position = '20 20'");
	TwDefine("Modelo valueswidth = 100 ");
	TwDefine("Modelo color = '42 46 148' alpha = 85");
	TwDefine("Modelo size = '300 400'");

	TwAddVarRW(modelInterface, "Escalar", TW_TYPE_FLOAT, &scale, "group='Transformaciones' min=0.01 step=0.01");
	TwAddVarRW(modelInterface, "Posici�n X", TW_TYPE_FLOAT, &translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(modelInterface, "Posici�n Y", TW_TYPE_FLOAT, &translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(modelInterface, "Posici�n Z", TW_TYPE_FLOAT, &translation[2], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(modelInterface, "Rotaci�n", TW_TYPE_QUAT4F, &rotation, "group='Transformaciones' opened=true");
	TwAddSeparator(modelInterface, NULL, "");
	TwAddVarRW(modelInterface, "Asimetr�a g", TW_TYPE_FLOAT, &asymmetry_param_g, "group='Par�metros de Dispersi�n' min=-1.0 max=1.0 step=0.01");
	TwAddVarRW(modelInterface, "�ndice de refracci�n", TW_TYPE_FLOAT, &refractive_index, "group = 'Par�metros de Dispersi�n' min=1.0 max=2.0 step = 0.01");
	{
		TwEnumVal material_type[5] = { { Patata, "Patata" }, { M�rmol, "M�rmol" }, { Piel, "Piel" }, { Leche, "Leche" }, { Ketchup, "Ketchup" } };
		TwType light = TwDefineEnum("material_type", material_type, 5);
		TwAddVarRW(modelInterface, "Material", light, &current_material, "group='Par�metros de Dispersi�n'");
	}
	TwAddSeparator(modelInterface, NULL, "");
	TwAddVarRW(modelInterface, "Shininess", TW_TYPE_FLOAT, &shininess, "group='Iluminaci�n' min=1.0 max=400.0 step=1.0");
}

interfaceModel::~interfaceModel()
{
}

void interfaceModel::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interfaceModel::show()
{
	TwDefine("Modelo visible = true");
}

void interfaceModel::hide()
{
	TwDefine("Modelo visible = false");
}