#pragma once
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

typedef enum { Scattered_Map, GBuffer_Light_Position, GBuffer_Light_Normal, GBuffer_Light_Depth } texture_t;

void TW_CALL set_model_scattering(const void *value, void *clientData);
void TW_CALL get_model_scattering(void *value, void *clientData);
void TW_CALL set_volume_scattering(const void *value, void *clientData);
void TW_CALL get_volume_scattering(void *value, void *clientData);
void TW_CALL set_model_center(const void *value, void *clientData);
void TW_CALL get_model_center(void *value, void *clientData);
void TW_CALL set_model_specular(const void *value, void *clientData);
void TW_CALL get_model_specular(void *value, void *clientData);

class interface_menu
{
private:
	static interface_menu *user_interface;
	TwBar *menu_interface;
	interface_menu();

public:
	int num_of_cameras, camera_selected;
	texture_t current_texture_type;

	static interface_menu * instance();
	~interface_menu();

	void reshape(int g_width, int g_height);
	void update_width(int g_width);
	void set_max_values(int num_of_cameras);
	void update_position();
	void show();
	void hide();
};