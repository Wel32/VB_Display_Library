
#include "menu_run.h"

#include "user_menu_screens.h"


menu_func_t PrevMenuPointer = NULL, CurrentMenuPointer = Screen1::Select;



void menu_task1(void *pvParameters)
{
	menu_func_t temp_ptr;

	while (1)
	{
		temp_ptr = (menu_func_t)(*CurrentMenuPointer)();

		if (temp_ptr == NULL) temp_ptr = PrevMenuPointer;
		PrevMenuPointer = CurrentMenuPointer;
		CurrentMenuPointer = temp_ptr;
	}
}

void menu_init()
{
	xTaskCreate(&menu_task1, "menu_task", 15000, NULL, 3, NULL);
}


