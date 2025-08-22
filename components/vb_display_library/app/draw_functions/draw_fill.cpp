#include "../display_core.h"
#include "draw_fill.h"



VBDL_Fill::VBDL_Fill(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1, uint32_t _color, uint8_t _options)
{
	if (_x1 > _x0)
	{
		pos.x0 = _x0;
		pos.x1 = _x1;
	}
	else
	{
		pos.x0 = _x1;
		pos.x1 = _x0;
	}
	
	if (_y1 > _y0)
	{
		pos.y0 = _y0;
		pos.y1 = _y1;
	}
	else
	{
		pos.y0 = _y1;
		pos.y1 = _y0;
	}

	//x0 = pos.x0;
	//y0 = pos.y0;
	
	color = _color;
	options = _options;
}

VBDL_Fill::VBDL_Fill(rect _pos, uint32_t _color, uint8_t _options)
{
	pos = _pos;
	color = _color;
	options = _options;
}
void VBDL_Fill::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	VBDL_InternalPushLayer img(this, mask, layer_options);
	if (!img.is_overlap()) return;

		if (img.obj_handle->options&DRAW_INV_COLOR) img.user_color = VBDisplay::invert_color(img.user_color);
	#if LCD_INV_BRIGHTNESS
		img.user_color = (img.user_color & 0xFF000000) | ((0xFFFFFFFF - img.user_color) & 0xFFFFFF);
	#endif
		
		uint8_t temp_trans = img.user_color >> 24;
		img.user_data = temp_trans * 256 / 255;
		
	#if LCD_SPI_ENABLE_DMA
		img.user_data |= ((img.layer_options&LAYER_OPTIONS_SINGLE_LAYER) ? 2 : 3)<<24;
	#else 
		img.user_data |= ((img.layer_options&LAYER_OPTIONS_SINGLE_LAYER) ? 1 : 2)<<24;
	#endif

	img.confirm(buf);
}
void VBDL_Fill::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
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
void VBDL_Fill::fill_str_memclear(internal_draw_obj* img)
{
		
}


