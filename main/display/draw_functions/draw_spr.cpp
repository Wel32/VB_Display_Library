#include "../display_core.h"
#include "draw_spr.h"



inline uint8_t conv_data_color(uint8_t data_color, uint8_t options)
{
	if (options&(CORR_GAMMA1 | CORR_GAMMA2))
	{
		uint8_t inv_option = options&(INVERT_CORR_GAMMA | DRAW_INV_TRANSPARENCY);
		if (inv_option == INVERT_CORR_GAMMA || inv_option == DRAW_INV_TRANSPARENCY) data_color = 240 - data_color;
		
		if (options&CORR_GAMMA1) data_color = inverted_tFont_gamma1[data_color];
		else data_color = inverted_tFont_gamma2[data_color];
		
		if (options&INVERT_CORR_GAMMA) data_color = 240 - data_color;
	}
	else if (options&DRAW_INV_TRANSPARENCY) data_color = 240 - data_color;
	
	return data_color;
}


void spr_str_init(internal_draw_obj* img)
{
	if (img->options&DRAW_INV_COLOR) img->user_color = invert_color(img->user_color);
	
#if LCD_INV_BRIGHTNESS
	img->user_color = (img->user_color & 0xFF000000) | ((0xFFFFFFFF - img->user_color) & 0xFFFFFF);
#endif
	
	uint32_t com_img_bias = calc_start_pos(img->img_cur_pos, img->img_cur_y, img->img_width_elem, 1);
  
	img->img_cur_pos = 0;
	uint8_t data_color = 0;
  
	uint8_t* sprite_data = (uint8_t*)(img->handle);
  
	for (uint32_t i = 0; i < com_img_bias; i++, img->img_cur_pos--)
	{
		if (!img->img_cur_pos)
		{
			data_color = (*sprite_data) & 0xF0;
			img->img_cur_pos = (*sprite_data) & 0x0F;
        
			sprite_data++;
			img->handle = (void*)sprite_data;
      
			if (!img->img_cur_pos)
			{
				img->img_cur_pos = (*sprite_data) + 16;
          
				sprite_data++;
				img->handle = (void*)sprite_data;
			}
		}
	}
  
	union {uint32_t full; uint16_t dc[2]; } mixdata;
	
	uint32_t trans = img->user_color >> 24;
	data_color = conv_data_color(data_color, img->options);
	mixdata.dc[1] = ((trans * 240 + (data_color * (255 - trans))) * 256) / (240 * 255);
	mixdata.dc[0] = 256 - mixdata.dc[1];    //256...0
	img->user_data = mixdata.full;
}


void spr_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	//uint16_t first_layer = img->options&DRAW_OPTIONS_FIRST_LAYER;
	
	uint8_t* sprite_data = (uint8_t*)(img->handle);

	union {uint32_t full; uint8_t color[4]; } spr_color;
	spr_color.full = img->user_color;
  
	union {uint32_t full; uint16_t dc[2]; } mixdata;
	mixdata.full = img->user_data;
  
	uint8_t data_color;
  
	for (uint32_t i = 0; i < img->img_width_elem; i++, img->img_cur_pos--)
	{
		if (!img->img_cur_pos)
		{
			data_color = (*sprite_data) & 0xF0;
			img->img_cur_pos = (*sprite_data) & 0x0F;
        
			sprite_data++;
			img->handle = (void*)sprite_data;

			uint32_t trans = spr_color.color[3];
			data_color = conv_data_color(data_color, img->options);
			mixdata.dc[1] = ((trans * 240 + (data_color * (255 - trans))) * 256) / (240 * 255);
			
			mixdata.dc[0] = 256 - mixdata.dc[1];    //256...0

			img->user_data = mixdata.full;
      
			if (!img->img_cur_pos)
			{
				img->img_cur_pos = (*sprite_data) + 16;
          
				sprite_data++;
				img->handle = (void*)sprite_data;
			}
		}

		if (i < img->buf_cnt_pix)
		{
			/*
			if (bmp_first_layer)
			{
				*buf = ((mixdata.dc[0]*spr_color.color[0]) + (mixdata.dc[1]*(255 - spr_color.color[0]))) >> 8;
				buf++;
				*buf = ((mixdata.dc[0]*spr_color.color[1]) + (mixdata.dc[1]*(255 - spr_color.color[1]))) >> 8;
				buf++;
				*buf = ((mixdata.dc[0]*spr_color.color[2]) + (mixdata.dc[1]*(255 - spr_color.color[2]))) >> 8;
				buf++;
			}
			else
			*/
			{
				*buf = ((mixdata.dc[0]*spr_color.color[0]) + (mixdata.dc[1]*(*buf))) >> 8;
				buf++;
				*buf = ((mixdata.dc[0]*spr_color.color[1]) + (mixdata.dc[1]*(*buf))) >> 8;
				buf++;
				*buf = ((mixdata.dc[0]*spr_color.color[2]) + (mixdata.dc[1]*(*buf))) >> 8;
				buf++;
			}
		}
  
	}
}

void spr_str_memclear(internal_draw_obj* img)
{
}




draw_obj make_sprite(const tSprite* spr, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
	if (spr == NULL) return make_void_obj();
	
	draw_obj res;
  
	res.pos = get_rect(x, y, spr->width, spr->height, align);
	res.obj_type = TSPRITE;
	res.handle = (void*)spr->data;
	res.color = color;
	res.options = options;
  
	return res;
}
