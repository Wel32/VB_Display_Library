#pragma once

#include "../user_drivers/system_include.h"
#include "../user_main.h"





typedef void* (*menu_func_t)();
typedef void* into_menu;


extern menu_func_t PrevMenuPointer, CurrentMenuPointer;



#if defined(__cplusplus)
extern "C" {
#endif
	
	
	void menu_init();
	
	
#if defined(__cplusplus)
}
#endif




