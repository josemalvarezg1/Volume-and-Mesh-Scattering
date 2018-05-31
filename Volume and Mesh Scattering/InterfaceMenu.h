#pragma once
#include <AntTweakBar.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>

class interface_menu
{
private:
	static interface_menu *user_interface;
	TwBar *menu_interface;
	interface_menu();

public:
	int num_of_cameras, camera_selected;

	static interface_menu * instance();
	~interface_menu();

	void reshape(int g_width, int g_height);
	void update_width(int g_width);
	void set_max_values(int num_of_cameras);
	void show();
	void hide();
};
