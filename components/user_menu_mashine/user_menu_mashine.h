#pragma once
#include "app_hal_api.h"



typedef void* (*user_menu_t)();
typedef void* into_menu;


extern user_menu_t PrevMenuPointer, CurrentMenuPointer;


#if defined(__cplusplus)
extern "C" {
#endif
	
	void user_menu_init(uint32_t stack_depth, user_menu_t entry_point);
	
#if defined(__cplusplus)
}
#endif











