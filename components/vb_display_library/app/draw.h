#pragma once


#include "display_datatypes.h"


#include "draw_functions/draw_fill.h"
#include "draw_functions/draw_bmp.h"
#include "draw_functions/draw_spr.h"
#include "draw_functions/draw_tFont.h"











typedef enum 
{
	TFILL,
	TBMP,
	TSPRITE,
	TFONT,
	
	
	
	TVOID,
}
drobj_type;




//DRAW_OPTIONS// 0...13 bits

#define DRAW_INV_COLOR (1<<0)
#define DRAW_INV_TRANSPARENCY (1<<1)


//#define CORR_GAMMA1 (1<<5)
#define CORR_GAMMA2 (1<<6)
#define INVERT_CORR_GAMMA (1<<7)























