
#include "menu_stack.h"




uint8_t MenuTreeStack::TempMenuLevel = 0;
menu_func_t MenuTreeStack::TempMenuPointer[10] = { NULL };
menu_func_t MenuTreeStack::PrevMenu = NULL;

void MenuTreeStack::SetRootMenu(menu_func_t val)
{
	PrevMenu = NULL;

	TempMenuLevel = 0;
	TempMenuPointer[TempMenuLevel] = val;
}
menu_func_t MenuTreeStack::GetRootMenu()
{
	return TempMenuPointer[0];
}
void MenuTreeStack::PushMenu(menu_func_t val)
{
	if (TempMenuPointer[TempMenuLevel] != NULL) PrevMenu = TempMenuPointer[TempMenuLevel];

	if (TempMenuLevel < ((sizeof(TempMenuPointer) / sizeof(menu_func_t)) - 1))
	{
		if (TempMenuPointer[TempMenuLevel] != val) TempMenuPointer[++TempMenuLevel] = val;
	}
}
menu_func_t MenuTreeStack::ReturnToRoot()
{
	if (TempMenuPointer[TempMenuLevel] != NULL) PrevMenu = TempMenuPointer[TempMenuLevel];

	TempMenuLevel = 0;

	return TempMenuPointer[TempMenuLevel];
}
menu_func_t MenuTreeStack::ResetTreeAndReturnToRoot()
{
	PrevMenu = NULL;

	TempMenuLevel = 0;

	return TempMenuPointer[TempMenuLevel];
}
menu_func_t MenuTreeStack::PopMenu()
{
	if (TempMenuPointer[TempMenuLevel] != NULL) PrevMenu = TempMenuPointer[TempMenuLevel];

	do
	{
		if (TempMenuLevel) TempMenuLevel--;
		else break;
	} while (TempMenuPointer[TempMenuLevel] == NULL); //use PushMenu(NULL) to implement additional features

	return TempMenuPointer[TempMenuLevel];
}
menu_func_t MenuTreeStack::GetCurrentMenu()
{
	return TempMenuPointer[TempMenuLevel];
}
menu_func_t MenuTreeStack::GetPrevMenu()
{
	return PrevMenu;
}

void MenuTreeStack::ForceSetPrevMenu(menu_func_t new_val)
{
	PrevMenu = new_val;
}
void MenuTreeStack::ForceSetCurrentMenu(menu_func_t new_val)
{
	TempMenuPointer[TempMenuLevel] = new_val;
}




