#include "../display_core.h"
#include "draw_bmp.h"



bmpInfo read_bmp_header(uint8_t * header)
{
	bmpInfo res;
  
	if ((header[0] != 0x42) || (header[1] != 0x4D)) goto err;

	res.imageOffset = header[10] | (header[11] << 8) |
	                       (header[12] << 16) | (header[13] << 24);
	res.imageWidth  = header[18] | (header[19] << 8) |
	                       (header[20] << 16) | (header[21] << 24);
	res.imageHeight = header[22] | (header[23] << 8) |
	                       (header[24] << 16) | (header[25] << 24);
	res.imagePlanes = header[26] | (header[27] << 8);

	res.imageBitsPerPixel = header[28] | (header[29] << 8);
	res.imageCompression  = header[30] | (header[31] << 8) |
	                             (header[32] << 16) |
	                             (header[33] << 24);
    

	if (res.imageWidth == 0 || res.imageHeight == 0 || res.imageWidth > 65536 || res.imageHeight > 65536) goto err;

	if ((res.imagePlanes != 1) ||
	   (res.imageCompression != 0)) goto err;
	
	
    
	return res;
  
err:
	res.imageWidth = 0;
	res.imageHeight = 0;
  
	return res;
}



inline i32_databytes_union get_rgb24_color_from_332(uint8_t clr332)
{
	i32_databytes_union ret;

	ret.b[0] = clr332&3; clr332>>=2;
	ret.b[0] = ret.b[0]*85;

	ret.b[1] = clr332&7; clr332>>=3;
	ret.b[1] = ret.b[1]*36 + (ret.b[1]>>1);

	ret.b[2] = clr332&7;
	ret.b[2] = ret.b[2]*36 + (ret.b[2]>>1);

	return ret;
}

inline i32_databytes_union get_rgb24_color_from_565(uint16_t clr565)
{
	i32_databytes_union ret;

	ret.b[0] = clr565&0x1F; clr565>>=5;
	ret.b[0] = ret.b[0]*8 + (ret.b[0]>>2);

	ret.b[1] = clr565&0x3F; clr565>>=6;
	ret.b[1] = ret.b[1]*4 + (ret.b[1]>>4);

	ret.b[2] = clr565&0x1F;
	ret.b[2] = ret.b[2]*8 + (ret.b[2]>>2);

	return ret;
}




void bmp_str_init(internal_draw_obj* img)
{
	uint8_t elem_per_pixel;
	
	switch (img->options&BMP_TYPE_MASK)
	{
	case BMP_ARGB32:
		elem_per_pixel = 4;
		break;
	case BMP_RGB24:
		elem_per_pixel = 3;
		break;
	case BMP_RGB16:
		elem_per_pixel = 2;
		break;
	case BMP_RGB8:
		elem_per_pixel = 1;
		break;
	default:
		img->obj_type = TVOID;
		return;
	}
	
	switch (img->options&BMP_SOURCE_MASK)
	{
	case BMP_FILE:
	case BMP_EMBED_BMP:
#ifdef USER_FILESYSTEM
	case BMP_FROM_USER_FS:
#endif
		if ((img->img_width_elem) & 0x03)
		{
			img->img_width_elem CLRBITS 0x03;
			img->img_width_elem += 4;
		}
		
		img->options ^= DRAW_INV_COLOR;
		break;
		
	case BMP_BMPC: break;
	default:
		img->obj_type = TVOID;
		return;
	}
	
	
	img->img_width_elem *= elem_per_pixel;
	if ((img->options & BMP_COLORTYPE_MASK) == BMP_INDEXED_4BIT) img->img_width_elem /= 2;

	if (img->options&BMP_VERTICAL_MIRROR) img->img_cur_pos = calc_start_pos_vertical_mirror(img->img_cur_pos, img->img_cur_y, img->img_width_elem, img->img_height, elem_per_pixel);
	else img->img_cur_pos = calc_start_pos(img->img_cur_pos, img->img_cur_y, img->img_width_elem, elem_per_pixel);
	

	
	
	switch (img->options&BMP_SOURCE_MASK)
	{
	case BMP_FILE:
		{
			if (img->options & BMP_COLORTYPE_MASK)
			{
				img->obj_type = TVOID;
				return;
			}

			const char *filename = (const char *) img->handle;
			FILE *file;
			file = fopen(filename, "r");
			if (file == NULL)
			{
				img->obj_type = TVOID;
				return;
			}
		
			alloc_additional_str_buffer();
	
			union {uint32_t full; uint8_t b[4]; } offset;
	
			fseek(file, 10, SEEK_SET);
			fread(&offset.b[0], sizeof(uint8_t), 4, file);
	
			fseek(file, img->img_cur_pos + offset.full, SEEK_SET);
			img->img_cur_pos += offset.full;
	
			img->handle = (FILE*) file;
			
			break;
		}
#ifdef USER_FILESYSTEM
	case BMP_FROM_USER_FS:
		{
			if (img->options & BMP_COLORTYPE_MASK)
			{
				img->obj_type = TVOID;
				return;
			}

			uint32_t data_size = img->img_height * img->img_width_elem;
			img->handle = (void*) user_open_bmp_source((USER_BMP_SOURCE)(img->handle), 0, data_size);
			
			if (img->handle == NULL)
			{
				img->obj_type = TVOID;
				return;
			}
			
			break;
		}
#endif
	}

	if (img->options & BMP_COLORTYPE_MASK)
	{
		img->user_data = 0;

		switch(img->options & BMP_COLORTYPE_MASK)
		{
			case BMP_INDEXED_8BIT:
			{
				img->img_cur_pos += 256*4;
				break;
			}
			case BMP_INDEXED_4BIT:
			{
				img->img_cur_pos += 16*4;
				break;
			}
			//case BMP_INDEXED_2BIT:
			//{
				//img->img_cur_pos += 4*4;
				//break;
			//}
		}
	}
	else
	{
		uint8_t temp_trans = img->user_color >> 24;
		img->user_data = temp_trans * 256 / 255;
	}
}




void internal_bmp_from_buf_str_memcpy(uint8_t* buf, internal_draw_obj* img, uint8_t* imgc, uint16_t read_elem_cnt)
{
	uint16_t bmp_type = img->options&BMP_TYPE_MASK;
	uint16_t bmp_inv_color = img->options&DRAW_INV_COLOR;
	uint16_t bmp_first_layer = img->layer_options&LAYER_OPTIONS_FIRST_LAYER;
	uint16_t bmp_chromakeyed = img->options&BMP_CHROMAKEYED;
	
	uint8_t back_color1, back_color2, back_color3;
	
	back_color1 = img->user_color;
	back_color2 = (img->user_color) >> 8;
	back_color3 = (img->user_color) >> 16;

	uint16_t user_transparency;
	uint16_t inv_user_transparency;

	if (img->options & BMP_COLORTYPE_MASK)
	{
		uint8_t temp_trans = img->user_color >> 24;
		user_transparency = temp_trans * 256 / 255;
	}
	else user_transparency = img->user_data;
	
	inv_user_transparency = 256 - user_transparency;

	uint16_t a16;
	uint16_t inv_a16;
	
	switch (bmp_type)
	{
	case BMP_RGB8:
		for (uint16_t j = 0; j < read_elem_cnt; j++)
		{
			uint8_t clr1, clr2, clr3;

			if (img->options & BMP_RGBA)
			{
				uint8_t a;

				
				if (img->options & BMP_COLORTYPE_MASK)
				{
					uint8_t dtb = *imgc;

					switch(img->options & BMP_COLORTYPE_MASK)
					{
						case BMP_INDEXED_8BIT:
						{
							imgc++;
							break;
						}
						case BMP_INDEXED_4BIT:
						{
							img->user_data = !img->user_data;

							if (img->user_data) dtb>>=4;
							else
							{
								dtb &= 0x0F;
								imgc++;
							}
							break;
						}
						//case BMP_INDEXED_2BIT:
						//{
							//break;
						//}
					}

					if (bmp_chromakeyed)
					{
						clr1 = back_color1;
						clr2 = back_color2;
						clr3 = back_color3;

						if (bmp_first_layer)
						{
							back_color1 = 255 - back_color1;
							back_color2 = 255 - back_color2;
							back_color3 = 255 - back_color3;
						}
					}
					else
					{
						clr1 = *((uint8_t*)img->handle + dtb*4);
						clr2 = *((uint8_t*)img->handle + dtb*4 + 1);
						clr3 = *((uint8_t*)img->handle + dtb*4 + 2);

						if (bmp_inv_color) cpp_swap(clr1, clr3);
					}

					a = *((uint8_t*)img->handle + dtb*4 + 3);
				}
				else if (bmp_chromakeyed)
				{
					clr1 = back_color1;
					clr2 = back_color2;
					clr3 = back_color3;

					if (bmp_first_layer)
					{
						back_color1 = 255 - back_color1;
						back_color2 = 255 - back_color2;
						back_color3 = 255 - back_color3;
					}

					a = *imgc++;
				}
				else
				{
					i32_databytes_union clr24 = get_rgb24_color_from_332(*imgc++);

					if (bmp_inv_color)
					{
						clr1 = clr24.b[0];
						clr2 = clr24.b[1];
						clr3 = clr24.b[2];
					}
					else
					{
						clr3 = clr24.b[0];
						clr2 = clr24.b[1];
						clr1 = clr24.b[2];
					}

					a = 0;
				}

				

				if (img->options&DRAW_INV_TRANSPARENCY)
				{
					inv_a16 = a * inv_user_transparency / 255;
					a16 = 256 - inv_a16;
				}
				else
				{
					a16 = a * inv_user_transparency / 255;
					inv_a16 = 256 - a16;
				}


				if (bmp_first_layer)
				{
					*buf = (a16 * clr1 + inv_a16 * back_color1) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * back_color2) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * back_color3) / 256;
					buf++;
				}
				else
				{
					*buf = (a16 * clr1 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * (*buf)) / 256;
					buf++;
				}
			}
			else
			{
				i32_databytes_union clr24 = get_rgb24_color_from_332(*imgc++);

				if (bmp_inv_color)
				{
					clr1 = clr24.b[0];
					clr2 = clr24.b[1];
					clr3 = clr24.b[2];
				}
				else
				{
					clr3 = clr24.b[0];
					clr2 = clr24.b[1];
					clr1 = clr24.b[2];
				}
			
		
#if LCD_INV_BRIGHTNESS
				clr1 = 255 - clr1;
				clr2 = 255 - clr2;
				clr3 = 255 - clr3;
#endif
				if (user_transparency)
				{
					if (bmp_first_layer)
					{
						*buf = (inv_user_transparency * clr1 + user_transparency * back_color1) / 256;
						buf++;
						*buf = (inv_user_transparency * clr2 + user_transparency * back_color2) / 256;
						buf++;
						*buf = (inv_user_transparency * clr3 + user_transparency * back_color3) / 256;
						buf++;
					}
					else
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
					*buf = clr1;
					buf++;
					*buf = clr2;
					buf++;
					*buf = clr3;
					buf++;
				}
			}
		}
		break;
  
	case BMP_RGB16:
		for (uint16_t j = 0; j < read_elem_cnt; j++)
		{
			uint8_t clr1, clr2, clr3;

			if (img->options & BMP_RGBA)
			{
				if (bmp_chromakeyed)
				{
					imgc++;

					clr1 = back_color1;
					clr2 = back_color2;
					clr3 = back_color3;

					if (bmp_first_layer)
					{
						back_color1 = 255 - back_color1;
						back_color2 = 255 - back_color2;
						back_color3 = 255 - back_color3;
					}
				}
				else
				{
					i32_databytes_union clr24 = get_rgb24_color_from_332(*imgc++);

					if (bmp_inv_color)
					{
						clr1 = clr24.b[0];
						clr2 = clr24.b[1];
						clr3 = clr24.b[2];
					}
					else
					{
						clr3 = clr24.b[0];
						clr2 = clr24.b[1];
						clr1 = clr24.b[2];
					}
				}

				uint8_t a = *imgc++;

				if (img->options&DRAW_INV_TRANSPARENCY)
				{
					inv_a16 = a * inv_user_transparency / 255;
					a16 = 256 - inv_a16;
				}
				else
				{
					a16 = a * inv_user_transparency / 255;
					inv_a16 = 256 - a16;
				}


				if (bmp_first_layer)
				{
					*buf = (a16 * clr1 + inv_a16 * back_color1) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * back_color2) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * back_color3) / 256;
					buf++;
				}
				else
				{
					*buf = (a16 * clr1 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * (*buf)) / 256;
					buf++;
				}
			}
			else
			{
				uint8_t clr5650 = *imgc++;
				uint8_t clr5651 = *imgc++;
				uint16_t clr565 = (clr5651<<8)|clr5650;
				i32_databytes_union clr24 = get_rgb24_color_from_565(clr565);

				if (bmp_inv_color)
				{
					clr1 = clr24.b[0];
					clr2 = clr24.b[1];
					clr3 = clr24.b[2];
				}
				else
				{
					clr3 = clr24.b[0];
					clr2 = clr24.b[1];
					clr1 = clr24.b[2];
				}
		
#if LCD_INV_BRIGHTNESS
				clr1 = 255 - clr1;
				clr2 = 255 - clr2;
				clr3 = 255 - clr3;
#endif
				if (user_transparency)
				{
					if (bmp_first_layer)
					{
						*buf = (inv_user_transparency * clr1 + user_transparency * back_color1) / 256;
						buf++;
						*buf = (inv_user_transparency * clr2 + user_transparency * back_color2) / 256;
						buf++;
						*buf = (inv_user_transparency * clr3 + user_transparency * back_color3) / 256;
						buf++;
					}
					else
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
					*buf = clr1;
					buf++;
					*buf = clr2;
					buf++;
					*buf = clr3;
					buf++;
				}
			}
		}
		break;
  
	case BMP_RGB24:
		for (uint16_t j = 0; j < read_elem_cnt; j++)
		{
			uint8_t clr1, clr2, clr3;

			if (img->options & BMP_RGBA)
			{
				if (bmp_chromakeyed)
				{
					imgc+=2;

					clr1 = back_color1;
					clr2 = back_color2;
					clr3 = back_color3;

					if (bmp_first_layer)
					{
						back_color1 = 255 - back_color1;
						back_color2 = 255 - back_color2;
						back_color3 = 255 - back_color3;
					}
				}
				else
				{
					uint8_t clr5650 = *imgc++;
					uint8_t clr5651 = *imgc++;
					uint16_t clr565 = (clr5651<<8)|clr5650;
					i32_databytes_union clr24 = get_rgb24_color_from_565(clr565);

					if (bmp_inv_color)
					{
						clr1 = clr24.b[0];
						clr2 = clr24.b[1];
						clr3 = clr24.b[2];
					}
					else
					{
						clr3 = clr24.b[0];
						clr2 = clr24.b[1];
						clr1 = clr24.b[2];
					}
				}

				uint8_t a = *imgc++;

				if (img->options&DRAW_INV_TRANSPARENCY)
				{
					inv_a16 = a * inv_user_transparency / 255;
					a16 = 256 - inv_a16;
				}
				else
				{
					a16 = a * inv_user_transparency / 255;
					inv_a16 = 256 - a16;
				}


				if (bmp_first_layer)
				{
					*buf = (a16 * clr1 + inv_a16 * back_color1) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * back_color2) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * back_color3) / 256;
					buf++;
				}
				else
				{
					*buf = (a16 * clr1 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * (*buf)) / 256;
					buf++;
				}
			}
			else
			{
				if (bmp_inv_color)
				{
					clr1 = *imgc++;
					clr2 = *imgc++;
					clr3 = *imgc++;
				}
				else
				{
					clr3 = *imgc++;
					clr2 = *imgc++;
					clr1 = *imgc++;
				}
			
		
#if LCD_INV_BRIGHTNESS
				clr1 = 255 - clr1;
				clr2 = 255 - clr2;
				clr3 = 255 - clr3;
#endif
				if (user_transparency)
				{
					if (bmp_first_layer)
					{
						*buf = (inv_user_transparency * clr1 + user_transparency * back_color1) / 256;
						buf++;
						*buf = (inv_user_transparency * clr2 + user_transparency * back_color2) / 256;
						buf++;
						*buf = (inv_user_transparency * clr3 + user_transparency * back_color3) / 256;
						buf++;
					}
					else
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
					*buf = clr1;
					buf++;
					*buf = clr2;
					buf++;
					*buf = clr3;
					buf++;
				}
			}
		}
		break;
  
	case BMP_ARGB32:
		{
			uint8_t clr1, clr2, clr3, a = 0;
			
			for (uint16_t j = 0; j < read_elem_cnt; j++)
			{
				if (!(img->options & BMP_RGBA)) a = *imgc++;

				if (bmp_chromakeyed)
				{
					imgc+=3;

					clr1 = back_color1;
					clr2 = back_color2;
					clr3 = back_color3;

					if (bmp_first_layer)
					{
						back_color1 = 255 - back_color1;
						back_color2 = 255 - back_color2;
						back_color3 = 255 - back_color3;
					}
				}
				else
				{
					if (bmp_inv_color)
					{
						clr1 = *imgc++;
						clr2 = *imgc++;
						clr3 = *imgc++;
					}
					else
					{
						clr3 = *imgc++;
						clr2 = *imgc++;
						clr1 = *imgc++;
					}
				}
			
				if (img->options & BMP_RGBA) a = *imgc++;
		
				
			
				if (img->options&DRAW_INV_TRANSPARENCY)
				{
					inv_a16 = a * inv_user_transparency / 255;
					a16 = 256 - inv_a16;
				}
				else
				{
					a16 = a * inv_user_transparency / 255;
					inv_a16 = 256 - a16;
				}
			
		
#if LCD_INV_BRIGHTNESS
				clr1 = 255 - clr1;
				clr2 = 255 - clr2;
				clr3 = 255 - clr3;
#endif
				
				if (bmp_first_layer)
				{
					*buf = (a16 * clr1 + inv_a16 * back_color1) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * back_color2) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * back_color3) / 256;
					buf++;
				}
				else
				{
					*buf = (a16 * clr1 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr2 + inv_a16 * (*buf)) / 256;
					buf++;
					*buf = (a16 * clr3 + inv_a16 * (*buf)) / 256;
					buf++;
				}
			}
		}
		break;
	}
	
	if (img->options&BMP_VERTICAL_MIRROR) img->img_cur_pos -= img->img_width_elem;
	else img->img_cur_pos += img->img_width_elem;
}

void bmp_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	uint8_t* imgc;
	uint16_t read_elem_cnt;
		
	switch (img->options&BMP_SOURCE_MASK)
	{
	case BMP_FILE:
		{
			FILE *file = (FILE*) img->handle;
			imgc = string_alloc_buffer;
			
			read_elem_cnt = fread(imgc, sizeof(uint8_t), img->buf_cnt_pix * 3, file);
			
			internal_bmp_from_buf_str_memcpy(buf, img, imgc, read_elem_cnt);
			
			if (img->options&BMP_VERTICAL_MIRROR) fseek(file, img->img_cur_pos, SEEK_SET);
			else
			{
				uint8_t elem_per_pixel;
				switch (img->options&BMP_TYPE_MASK)
				{
				case BMP_RGB24:
					elem_per_pixel = 3;
					break;
					/*
				case BMP_ARGB32:
					elem_per_pixel = 4;
					break;
					*/
				default: elem_per_pixel = 4;
				}
				
				fseek(file, img->img_width_elem - img->buf_cnt_pix * elem_per_pixel, SEEK_CUR);
			}
			break;
		}
#ifdef USER_FILESYSTEM
	case BMP_FROM_USER_FS:
		{
			read_elem_cnt = img->buf_cnt_pix;
			imgc = (uint8_t*)user_get_bmp_addr(img->handle, img->img_cur_pos, read_elem_cnt);
		
			internal_bmp_from_buf_str_memcpy(buf, img, imgc, read_elem_cnt);
			break;
		}
#endif
	default:
		imgc = ((uint8_t*)(img->handle)) + (img->img_cur_pos);
		read_elem_cnt = img->buf_cnt_pix;
		
		internal_bmp_from_buf_str_memcpy(buf, img, imgc, read_elem_cnt);
	}
}



void bmp_str_memclear(internal_draw_obj* img)
{
	switch (img->options&BMP_SOURCE_MASK)
	{
	case BMP_FILE:
		{
			FILE *file = (FILE*) img->handle;
			fclose(file);
			
			break;
		}
#ifdef USER_FILESYSTEM
	case BMP_FROM_USER_FS:
		{
			user_fclose((USER_FILE**)&img->handle);
			
			break;
		}
#endif
	}
}










draw_obj make_bmp_from_tImage(const tImage* img, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
	draw_obj res = make_void_obj();
	if (img == NULL) return res;
	
	res.obj_type = TBMP;
	res.options = options | BMP_BMPC;
	
	if (img->dataSize == 24) res.options SETBITS BMP_RGB24;
	else if (img->dataSize == 32) res.options SETBITS BMP_ARGB32;
	else return make_void_obj();
  
	res.pos = get_rect(x, y, img->width, img->height, align);
	
	res.handle = (void*)img->data;
	res.color = color;
  
	return res;
}



draw_obj make_embed_bmp(const char* bmpfile_start_pointer, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
	draw_obj res = make_void_obj();
	if (bmpfile_start_pointer == NULL) return res;
	
	bmpInfo bmp_info = read_bmp_header((uint8_t*)bmpfile_start_pointer);
	
	res.obj_type = TBMP;
	res.options = options | BMP_EMBED_BMP;
	
	if (bmp_info.imageBitsPerPixel == 24) res.options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) res.options SETBITS BMP_ARGB32;
	else return make_void_obj();
  
	res.pos = get_rect(x, y, bmp_info.imageWidth, bmp_info.imageHeight, align);
	
	res.handle = (void*)(bmpfile_start_pointer + bmp_info.imageOffset);
	res.color = color;
  
	return res;
}



draw_obj make_file_bmp_from_file(const char* filename, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
	draw_obj res = make_void_obj();
	res.handle = (void*)filename;
	
	FILE *file;
	file = fopen(filename, "r");
	if (file == NULL) return make_void_obj();
	
	res.obj_type = TBMP;
	res.options = options | BMP_FILE;
	
	uint8_t header[34];
	
	rewind(file);
	fread(header, sizeof(uint8_t), 34, file);
	
	fclose(file);
	
	bmpInfo bmp_info = read_bmp_header(header);
	
	if (bmp_info.imageBitsPerPixel == 24) res.options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) res.options SETBITS BMP_ARGB32;
	else return make_void_obj();
  
	res.pos = get_rect(x, y, bmp_info.imageWidth, bmp_info.imageHeight, align);
	
	res.color = color;
  
	return res;
}





#ifdef USER_FILESYSTEM
draw_obj make_bmp_from_user_filesystem(const char *file_name, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align)
{
	draw_obj res = make_void_obj();
	USER_FILE outfile;
	
	if (!user_fopen(file_name, &outfile))
	{
		ESP_LOGI("BMP", "file not found %s", file_name);
		return res;
	}
	
	size_t handle_offset = (size_t)outfile.addr;
	
	outfile.addr = (const void*) user_open_bmp_source(handle_offset, 0, 34);
	bmpInfo bmp_info = read_bmp_header((uint8_t*)user_get_bmp_addr(outfile.addr, 0, 34));
	
	res.obj_type = TBMP;
	res.options = options | BMP_FROM_USER_FS;
	
	if (bmp_info.imageBitsPerPixel == 24) res.options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) res.options SETBITS BMP_ARGB32;
	else
	{
		ESP_LOGI("BMP", "format is not supported %s", file_name);
		return make_void_obj();
	}
  
	res.pos = get_rect(x, y, bmp_info.imageWidth, bmp_info.imageHeight, align);
	
	res.handle = (void*)(handle_offset + bmp_info.imageOffset);
	res.color = color;
	
	user_fclose_source((USER_FILE)res.handle);
  
	return res;
}
#endif

