#pragma once

#include "../user_drivers/system_include.h"
#include "../user_drivers/user_lcd.h"
#include "display_options.h"


#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2

	#define ScreenWidth _ScreenWidth
	#define ScreenHeight _ScreenHeight

	#if ORIENTATIONS_COUNT == 4

	#define PortraitOrientation (ScreenOrient&1)

	#else

		#define LCD_PORTRAIT LCD_PORTRAIT1
		#define LCD_ALBUM LCD_ALBUM1

		#define PortraitOrientation ScreenOrient

	#endif

	#define ORIENTATION_DIR(por, al) ((ScreenOrient)?(por):(al))

#else

	#define ScreenWidth (DISPLAY_WIDTH)
	#define ScreenHeight (DISPLAY_HEIGHT)

#endif



#define CENTER_ALIGN 0

#define LEFT_ALIGN 1
#define RIGHT_ALIGN 2

#define BOTTOM_ALIGN (1<<4)
#define TOP_ALIGN (2<<4)

#define DELETE_IMG (255<<24)