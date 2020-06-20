#include "display_core.h"
#include "draw.h"






void(*str_init_func[TVOID])(internal_draw_obj* img) =
{
	fill_str_init,
	bmp_str_init,
	spr_str_init,
	tFont_str_init,
};


void(*str_memcpy_func[TVOID])(uint8_t* buf, internal_draw_obj* img) =
{
	fill_str_memcpy,
	bmp_str_memcpy,
	spr_str_memcpy,
	tFont_str_memcpy,
};


void(*str_memclear_func[TVOID])(internal_draw_obj* img) =
{
	fill_str_memclear,
	bmp_str_memclear,
	spr_str_memclear,
	tFont_str_memclear,
};

