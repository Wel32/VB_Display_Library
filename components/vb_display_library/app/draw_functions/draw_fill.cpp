#include "../display_core.h"
#include "draw_fill.h"




void fill_str_init(internal_draw_obj* img)
{
	if (img->options&DRAW_INV_COLOR) img->user_color = invert_color(img->user_color);
#if LCD_INV_BRIGHTNESS
	img->user_color = (img->user_color & 0xFF000000) | ((0xFFFFFFFF - img->user_color) & 0xFFFFFF);
#endif
	
	uint8_t temp_trans = img->user_color >> 24;
	img->user_data = temp_trans * 256 / 255;
	
#if LCD_SPI_ENABLE_DMA
	img->user_data |= ((img->layer_options&LAYER_OPTIONS_SINGLE_LAYER) ? 2 : 3)<<24;
#else 
	img->user_data |= ((img->layer_options&LAYER_OPTIONS_SINGLE_LAYER) ? 1 : 2)<<24;
#endif
}


void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	if (img->user_data < (1 << 24)) return;
	
	uint8_t layer_count_options = img->user_data >> 24;
	
	uint16_t user_transparency, inv_user_transparency;
	
	if (!(img->layer_options&LAYER_OPTIONS_FIRST_LAYER))
	{
		user_transparency = (img->user_data) & 0xFFFF;
		inv_user_transparency = 256 - user_transparency;
	}
	else user_transparency = 0;

#if LCD_SPI_ENABLE_DMA
	if (layer_count_options && layer_count_options <= 2) img->user_data -= (1 << 24);
#else
	if (layer_count_options == 1) img->user_data -= (1 << 24);
#endif
	
	uint8_t clr1, clr2, clr3;
	
	clr1 = img->user_color;
	clr2 = (img->user_color) >> 8;
	clr3 = (img->user_color) >> 16;
	
  
	if (user_transparency)
	{
		for (uint16_t j = 0; j < img->buf_cnt_pix; j++)
		{
			*buf = (inv_user_transparency * clr1 + user_transparency * (*buf)) / 256;
			buf++;
			*buf = (inv_user_transparency * clr2 + user_transparency * (*buf)) / 256;
			buf++;
			*buf = (inv_user_transparency * clr3 + user_transparency * (*buf)) / 256;
			buf++;
		}
	}
	else
	{
		for (uint16_t j = 0; j < img->buf_cnt_pix; j++)
		{
			*buf ++= clr1;
			*buf ++= clr2;
			*buf ++= clr3;
		}
	}
}



void fill_str_memclear(internal_draw_obj* img)
{
}





draw_obj make_fill(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, uint8_t options)
{
	draw_obj res = make_void_obj();
	
	if (x1 > x0)
	{
		res.pos.x0 = x0;
		res.pos.x1 = x1;
	}
	else
	{
		res.pos.x0 = x1;
		res.pos.x1 = x0;
	}
	
	if (y1 > y0)
	{
		res.pos.y0 = y0;
		res.pos.y1 = y1;
	}
	else
	{
		res.pos.y0 = y1;
		res.pos.y1 = y0;
	}
	
	res.obj_type = TFILL;
	res.handle = NULL;
	res.color = color;
	res.options = options;
  
	return res;
}



