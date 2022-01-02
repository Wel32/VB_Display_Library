#pragma once

#include "user_menu_mashine.h"



class MenuTreeStack
{
	static vector <user_menu_t> MenuStackPtr;
	static user_menu_t PrevMenu;
	static int8_t MoveStepsCnt;
	static uint8_t PinnedLevel;
public:
	
	static uint8_t GetMenuLevel();
	static int8_t GetMenuMoveStepsCnt();
	static void SetRootMenu(user_menu_t val);
	static void PushMenu(user_menu_t val);
	static user_menu_t GetRootMenu();
	static user_menu_t ReturnToRoot();
	static user_menu_t PopMenu();
	static user_menu_t GetCurrentMenu();
	static user_menu_t GetPrevMenu();
	/////////////////////////////////////////////////////
	static void ForceSetPrevMenu(user_menu_t new_val);
	static void ForceSetCurrentMenu(user_menu_t new_val);
	/////////////////////////////////////////////////////
	static void PinNextLevelAsRoot();
	static uint8_t GetPinnedLevel();
	static void UnpinCurrentLevelFromRoot();
	static user_menu_t ReturnToRootOrPinned();
};
