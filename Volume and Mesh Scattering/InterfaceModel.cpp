#include "InterfaceModel.h"

interface_model * interface_model::user_interface = NULL;

interface_model * interface_model::instance()
{
	if (!user_interface)
		user_interface = new interface_model();

	return user_interface;
}

interface_model::interface_model()
{
	this->model_interface = TwNewBar("Modelo");
	TwDefine("Modelo refresh = '0.0001f'");
	TwDefine("Modelo resizable = false");
	TwDefine("Modelo fontresizable = false");
	TwDefine("Modelo movable = false");
	TwDefine("Modelo visible = false");
	TwDefine("Modelo position = '20 20'");
	TwDefine("Modelo valueswidth = 100 ");
	TwDefine("Modelo color = '42 46 148' alpha = 85");
	TwDefine("Modelo size = '300 400'");

	TwAddVarRW(this->model_interface, "Escalar", TW_TYPE_FLOAT, &this->scale, "group='Transformaciones' min=0.01 step=0.01");
	TwAddVarRW(this->model_interface, "Posición X", TW_TYPE_FLOAT, &this->translation[0], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->model_interface, "Posición Y", TW_TYPE_FLOAT, &this->translation[1], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->model_interface, "Posición Z", TW_TYPE_FLOAT, &this->translation[2], "group = 'Transformaciones' step = 0.01");
	TwAddVarRW(this->model_interface, "Rotación", TW_TYPE_QUAT4F, &this->rotation, "group='Transformaciones' opened=true");
	TwAddSeparator(this->model_interface, NULL, "");
	TwAddVarRW(this->model_interface, "Asimetría g", TW_TYPE_FLOAT, &this->asymmetry_param_g, "group='Parámetros de Dispersión' min=-1.0 max=0.99 step=0.01");
	{
		TwEnumVal material_type[6] = { { Patata, "Patata" }, { Mármol, "Mármol" }, { Piel, "Piel" }, { Leche, "Leche" }, { Crema, "Crema" }, { Ninguno, "Ninguno" } };
		TwType light = TwDefineEnum("material_type", material_type, 6);
		TwAddVarRW(this->model_interface, "Material", light, &this->current_material, "group='Parámetros de Dispersión'");
	}
	TwAddVarRW(this->model_interface, "Radio", TW_TYPE_FLOAT, &this->radius, "group='Parámetros de Dispersión' min=0.5 max=1.0 step=0.001");
	TwAddVarRW(this->model_interface, "Bias", TW_TYPE_FLOAT, &this->bias, "group='Parámetros de Dispersión' min=0.001 max=1.0 step=0.001");
	TwAddVarRW(this->model_interface, "Epsilon", TW_TYPE_FLOAT, &this->epsilon, "group='Parámetros de Dispersión' min=0.001 max=1.0 step=0.001");
}

interface_model::~interface_model()
{
}

void interface_model::reshape(int g_width, int g_height)
{
	TwWindowSize(g_width, g_height);
}

void interface_model::show()
{
	TwDefine("Modelo visible = true");
}

void interface_model::hide()
{
	TwDefine("Modelo visible = false");
}