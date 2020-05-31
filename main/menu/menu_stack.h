#pragma once

#include "menu_run.h"




class MenuTreeStack
{
	static menu_func_t TempMenuPointer[10];
	static uint8_t TempMenuLevel;
	static menu_func_t PrevMenu;
public:
	
	static void SetRootMenu(menu_func_t val);
	static void PushMenu(menu_func_t val);
	static menu_func_t GetRootMenu();
	static menu_func_t ReturnToRoot();
	static menu_func_t PopMenu();
	static menu_func_t GetCurrentMenu();
	static menu_func_t GetPrevMenu();
	static menu_func_t ResetTreeAndReturnToRoot();
	
	static void ForceSetPrevMenu(menu_func_t new_val);
	static void ForceSetCurrentMenu(menu_func_t new_val);
};
