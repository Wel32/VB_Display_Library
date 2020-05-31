#pragma once

#include "../user_drivers/system_include.h"
#include "../user_main.h"





#if defined(__cplusplus)
extern "C" {
#endif
	
	


typedef struct
{
	uint8_t ScreenBrightness;
	uint8_t ScreenOrient; //1 - portrait, 0 - landscape
	uint8_t HideDrawingLayers;
} UserSettingsStruct2;
	

	
	
	
void UserSettingsInit();
	
	
	
#if defined(__cplusplus)
}
#endif