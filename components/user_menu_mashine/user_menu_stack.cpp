
#include "user_menu_stack.hpp"


vector <user_menu_t> MenuTreeStack::MenuStackPtr(1);
user_menu_t MenuTreeStack::PrevMenu = NULL;
int8_t MenuTreeStack::MoveStepsCnt = 0;
uint8_t MenuTreeStack::PinnedLevel = 0;

uint8_t MenuTreeStack::GetMenuLevel()
{
	if (MenuStackPtr.size() == 0) return 0;
	return MenuStackPtr.size() - 1;
}
int8_t MenuTreeStack::GetMenuMoveStepsCnt()
{
	return MoveStepsCnt;
}
void MenuTreeStack::SetRootMenu(user_menu_t val)
{
	PrevMenu = NULL;
	MenuStackPtr.resize(1);
	MenuStackPtr.back() = val;
	MoveStepsCnt = 0;
	PinnedLevel = 0;
}
user_menu_t MenuTreeStack::GetRootMenu()
{
	return MenuStackPtr[0];
}
void MenuTreeStack::PushMenu(user_menu_t val)
{
	if (val == NULL) PinNextLevelAsRoot();
	else if (MenuStackPtr.back() != val && MenuStackPtr.size() < 255)
	{
		PrevMenu = MenuStackPtr.back();
		MenuStackPtr.push_back(val);
		MoveStepsCnt = + 1;
	}
}
user_menu_t MenuTreeStack::PopMenu()
{
	if (MenuStackPtr.size() > 1)
	{
		PrevMenu = MenuStackPtr.back();
		MenuStackPtr.pop_back();
		MoveStepsCnt = -1;
	}
	
	if (PinnedLevel >= MenuStackPtr.size()) PinnedLevel = 0;
	
	return MenuStackPtr.back();
}
user_menu_t MenuTreeStack::ReturnToRoot()
{
	MoveStepsCnt = 1 - MenuStackPtr.size();
	MenuStackPtr.resize(1);
	PinnedLevel = 0;
	return MenuStackPtr[0];
}
user_menu_t MenuTreeStack::GetCurrentMenu()
{
	return MenuStackPtr.back();
}
user_menu_t MenuTreeStack::GetPrevMenu()
{
	return PrevMenu;
}


////////////////////////////////////////////////////////////////


void MenuTreeStack::ForceSetPrevMenu(user_menu_t new_val)
{
	PrevMenu = new_val;
}
void MenuTreeStack::ForceSetCurrentMenu(user_menu_t new_val)
{
	MenuStackPtr.back() = new_val;
}


/////////////////////////////////////////////////////////////////

void MenuTreeStack::PinNextLevelAsRoot()
{
	PinnedLevel = MenuStackPtr.size();
}
uint8_t MenuTreeStack::GetPinnedLevel()
{
	return PinnedLevel;
}

void MenuTreeStack::UnpinCurrentLevelFromRoot()
{
	PinnedLevel = 0;
}
user_menu_t MenuTreeStack::ReturnToRootOrPinned()
{
	PrevMenu = MenuStackPtr.back();
	MenuStackPtr.resize(PinnedLevel + 1);
	MoveStepsCnt = MenuStackPtr.size() - PinnedLevel - 1;
	return MenuStackPtr.back();
}


