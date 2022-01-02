
#include "user_menu_mashine.h"



user_menu_t PrevMenuPointer, CurrentMenuPointer;


void user_menu_task(void *pvParameters)
{
	user_menu_t temp_ptr;

	while (1)
	{
		temp_ptr = (user_menu_t)(*CurrentMenuPointer)();

		if (temp_ptr == NULL) temp_ptr = PrevMenuPointer;
		PrevMenuPointer = CurrentMenuPointer;
		CurrentMenuPointer = temp_ptr;
	}
}

void user_menu_init(uint32_t stack_depth, user_menu_t entry_point)
{
	PrevMenuPointer = CurrentMenuPointer = entry_point;
	xTaskCreate(&user_menu_task, "menu_task", stack_depth, NULL, 3, NULL);
}