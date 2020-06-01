#pragma once


#define ORIENTATIONS_COUNT 4 //1,2,4 is acceptable
//#define ENABLE_FRAGMEMT_DRAWING





#include "display_macro.h"


#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2

	#define INIT_SCREEN_ORIENT LCD_PORTRAIT1
	#define DISPLAY_LONG_SIDE_SIZE 480
	#define DISPLAY_SHORT_SIDE_SIZE 320

#else	#define DISPLAY_WIDTH 320	#define DISPLAY_HEIGHT 480#endif


#define MAX_LAYERS_TO_BLENDING 20
#define LCD_INV_BRIGHTNESS 0



