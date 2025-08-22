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



bool bmp_common_str_init1(internal_draw_obj* img, bool align_4bits)
{
	uint8_t elem_per_pixel;
	
	switch (img->obj_handle->options&BMP_TYPE_MASK)
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
		return 0;
	}
	
	if (align_4bits)
	{
		if ((img->img_width_elem) & 0x03)
		{
			img->img_width_elem CLRBITS 0x03;
			img->img_width_elem += 4;
		}
	}
	
	img->img_width_elem *= elem_per_pixel;
	if ((img->obj_handle->options & BMP_COLORTYPE_MASK) == BMP_INDEXED_4BIT) img->img_width_elem /= 2;

	if (img->obj_handle->options&BMP_VERTICAL_MIRROR) img->img_cur_pos = VBDL_InternalPushLayer::calc_start_pos_vertical_mirror(img->img_cur_pos, img->img_cur_y, img->img_width_elem, img->img_height, elem_per_pixel);
	else img->img_cur_pos = VBDL_InternalPushLayer::calc_start_pos(img->img_cur_pos, img->img_cur_y, img->img_width_elem, elem_per_pixel);

	return 1;
}

void bmp_common_str_init2(internal_draw_obj* img)
{
	if (img->obj_handle->options & BMP_COLORTYPE_MASK)
	{
		img->user_data = 0;

		switch(img->obj_handle->options & BMP_COLORTYPE_MASK)
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


void internal_bmp_from_buf_str_memcpy_v2(uint8_t* buf, internal_draw_obj* img, uint8_t* imgc, uint16_t read_elem_cnt)
{
	uint16_t bmp_type = img->obj_handle->options&BMP_TYPE_MASK;
	uint16_t bmp_inv_color = img->obj_handle->options&DRAW_INV_COLOR;
	uint16_t bmp_first_layer = img->layer_options&LAYER_OPTIONS_FIRST_LAYER;
	uint16_t bmp_chromakeyed = img->obj_handle->options&BMP_CHROMAKEYED;
	
	uint8_t back_color1, back_color2, back_color3;
	
	back_color1 = img->user_color;
	back_color2 = (img->user_color) >> 8;
	back_color3 = (img->user_color) >> 16;

	uint16_t user_transparency;
	uint16_t inv_user_transparency;

	if (img->obj_handle->options & BMP_COLORTYPE_MASK)
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

			if (img->obj_handle->options & BMP_RGBA)
			{
				uint8_t a;

				
				if (img->obj_handle->options & BMP_COLORTYPE_MASK)
				{
					uint8_t dtb = *imgc;

					switch(img->obj_handle->options & BMP_COLORTYPE_MASK)
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
						clr1 = *((uint8_t*)img->temp_payload + dtb*4);
						clr2 = *((uint8_t*)img->temp_payload + dtb*4 + 1);
						clr3 = *((uint8_t*)img->temp_payload + dtb*4 + 2);

						if (bmp_inv_color) cpp_swap(clr1, clr3);
					}

					a = *((uint8_t*)img->temp_payload + dtb*4 + 3);
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

				

				if (img->obj_handle->options&DRAW_INV_TRANSPARENCY)
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

			if (img->obj_handle->options & BMP_RGBA)
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

				if (img->obj_handle->options&DRAW_INV_TRANSPARENCY)
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

			if (img->obj_handle->options & BMP_RGBA)
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

				if (img->obj_handle->options&DRAW_INV_TRANSPARENCY)
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
				if (!(img->obj_handle->options & BMP_RGBA)) a = *imgc++;

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
			
				if (img->obj_handle->options & BMP_RGBA) a = *imgc++;
		
				
			
				if (img->obj_handle->options&DRAW_INV_TRANSPARENCY)
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
	
	if (img->obj_handle->options&BMP_VERTICAL_MIRROR) img->img_cur_pos -= img->img_width_elem;
	else img->img_cur_pos += img->img_width_elem;
}


VBDL_tImage::VBDL_tImage(const tImage* img, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align)
{
	if (img == NULL) return;
	
	options = _options;
	
	if (img->dataSize == 24) options SETBITS BMP_RGB24;
	else if (img->dataSize == 32) options SETBITS BMP_ARGB32;
	else return;
  
	pos = VBDisplay::get_rect(_x, _y, img->width, img->height, _align);
	handle = (void*)img->data;
	color = _color;
}
void VBDL_tImage::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	VBDL_InternalPushLayer img(this, mask, layer_options);

	if (!img.is_overlap()) return;
	if (!bmp_common_str_init1(&img, 0)) return;
	img.temp_payload = handle;
	bmp_common_str_init2(&img);

	img.confirm(buf);
}
void VBDL_tImage::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	uint8_t* imgc;
	uint16_t read_elem_cnt;

	imgc = ((uint8_t*)(img->temp_payload)) + (img->img_cur_pos);
	read_elem_cnt = img->buf_cnt_pix;
	
	internal_bmp_from_buf_str_memcpy_v2(buf, img, imgc, read_elem_cnt);
}
void VBDL_tImage::fill_str_memclear(internal_draw_obj* img)
{
		
}








VBDL_Embed_BMP::VBDL_Embed_BMP(const char* bmpfile_start_pointer, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align)
{
	if (bmpfile_start_pointer == NULL) return;
	
	bmpInfo bmp_info = read_bmp_header((uint8_t*)bmpfile_start_pointer);
	
	options = _options^DRAW_INV_COLOR;
	
	if (bmp_info.imageBitsPerPixel == 24) options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) options SETBITS BMP_ARGB32;
	else return;
  
	pos = VBDisplay::get_rect(_x, _y, bmp_info.imageWidth, bmp_info.imageHeight, _align);
	handle = (void*)(bmpfile_start_pointer + bmp_info.imageOffset);
	color = _color;
}
void VBDL_Embed_BMP::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	VBDL_InternalPushLayer img(this, mask, layer_options);

	if (!img.is_overlap()) return;
	if (!bmp_common_str_init1(&img, 1)) return;
	img.temp_payload = handle;
	bmp_common_str_init2(&img);

	img.confirm(buf);
}
void VBDL_Embed_BMP::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	uint8_t* imgc;
	uint16_t read_elem_cnt;

	imgc = ((uint8_t*)(img->temp_payload)) + (img->img_cur_pos);
	read_elem_cnt = img->buf_cnt_pix;
	
	internal_bmp_from_buf_str_memcpy_v2(buf, img, imgc, read_elem_cnt);
}
void VBDL_Embed_BMP::fill_str_memclear(internal_draw_obj* img)
{
		
}







VBDL_BMP_from_file::VBDL_BMP_from_file(const char* filename, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align)
{
	FILE *file;
	file = fopen(filename, "r");
	if (file == NULL) return;
	
	options = _options^DRAW_INV_COLOR;
	
	uint8_t header[34];
	
	rewind(file);
	fread(header, sizeof(uint8_t), 34, file);
	
	fclose(file);
	
	bmpInfo bmp_info = read_bmp_header(header);
	
	if (bmp_info.imageBitsPerPixel == 24) options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) options SETBITS BMP_ARGB32;
	else return;
  
	pos = VBDisplay::get_rect(_x, _y, bmp_info.imageWidth, bmp_info.imageHeight, _align);
	handle = (void*)filename;
	color = _color;
}
void VBDL_BMP_from_file::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	VBDL_InternalPushLayer img(this, mask, layer_options);

	if (!img.is_overlap()) return;
	if(!bmp_common_str_init1(&img, 1)) return;
	img.temp_payload = handle;

	if (img.obj_handle->options & BMP_COLORTYPE_MASK) return;

	const char *filename = (const char *) img.temp_payload;
	FILE *file;
	file = fopen(filename, "r");
	if (file == NULL)
	{
		img.temp_payload = NULL;
		return;
	}
		
	//VBDisplay::alloc_additional_str_buffer();
	
	union {uint32_t full; uint8_t b[4]; } offset;
	
	fseek(file, 10, SEEK_SET);
	fread(&offset.b[0], sizeof(uint8_t), 4, file);
	
	fseek(file, img.img_cur_pos + offset.full, SEEK_SET);
	img.img_cur_pos += offset.full;
	
	img.temp_payload = (FILE*) file;

	bmp_common_str_init2(&img);

	img.confirm(buf);
}
void VBDL_BMP_from_file::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	if (img->temp_payload == NULL) return;
	
	uint8_t* imgc;
	uint16_t read_elem_cnt;

	imgc = ((uint8_t*)(img->temp_payload)) + (img->img_cur_pos);
	read_elem_cnt = img->buf_cnt_pix;
	
	internal_bmp_from_buf_str_memcpy_v2(buf, img, imgc, read_elem_cnt);
}
void VBDL_BMP_from_file::fill_str_memclear(internal_draw_obj* img)
{
	if (img->temp_payload == NULL) return;

	FILE *file = (FILE*) img->temp_payload;
	fclose(file);
}




#ifdef USER_FILESYSTEM

VBDL_BMP_from_user_filesystem::VBDL_BMP_from_user_filesystem(ufs_partition_t* partition, const char* file_name, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align)
{
	USER_FILE outfile;
	
	if (!user_fopen(partition, file_name, &outfile))
	{
		ESP_LOGI("BMP", "file not found %s", file_name);
		return;
	}
	
	USER_FILE img_data = {partition, NULL, outfile.offset, 34};
	outfile.addr = (const void*) user_open_obj_source(&img_data, 0, 0);
	bmpInfo bmp_info = read_bmp_header((uint8_t*)user_get_obj_addr(outfile.addr, 0, 34));
	
	options = _options^DRAW_INV_COLOR;
	
	if (bmp_info.imageBitsPerPixel == 24) options SETBITS BMP_RGB24;
	else if (bmp_info.imageBitsPerPixel == 32) options SETBITS BMP_ARGB32;
	else
	{
		ESP_LOGI("BMP", "format is not supported %s", file_name);
		user_fclose_source(img_data);
		return;
	}
  
	pos = VBDisplay::get_rect(_x, _y, bmp_info.imageWidth, bmp_info.imageHeight, _align);

	uint32_t img_width_elem = bmp_info.imageWidth * (bmp_info.imageBitsPerPixel/8);
	if (img_width_elem&3)
	{
		img_width_elem CLRBITS 3;
		img_width_elem += 4;
	}

	img_data.offset += bmp_info.imageOffset;
	img_data.size = img_width_elem * bmp_info.imageHeight;

	handle = malloc(sizeof(ufs_file));

	*(ufs_file*)handle = img_data;
	color = _color;
	
	user_fclose_source((USER_BMP_SOURCE) handle);
}
VBDL_BMP_from_user_filesystem::~VBDL_BMP_from_user_filesystem()
{
	if (handle != NULL) free(handle);
}
void VBDL_BMP_from_user_filesystem::fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options)
{
	if (handle == NULL) return;
	VBDL_InternalPushLayer img(this, mask, layer_options);

	if (!img.is_overlap()) return;
	if(!bmp_common_str_init1(&img, 1)) return;
	img.temp_payload = handle;

	if (img.obj_handle->options & BMP_COLORTYPE_MASK) return;
	
	uint32_t data_size = img.img_height * img.img_width_elem;
	img.temp_payload = (void*) user_open_obj_source((USER_BMP_SOURCE)(img.temp_payload), 0, data_size);
				
	if (img.temp_payload == NULL) return;
	
	bmp_common_str_init2(&img);

	img.confirm(buf);
}
void VBDL_BMP_from_user_filesystem::fill_str_memcpy(uint8_t* buf, internal_draw_obj* img)
{
	if (img->temp_payload == NULL) return;

	uint8_t* imgc;
	uint16_t read_elem_cnt;

	read_elem_cnt = img->buf_cnt_pix;
	imgc = (uint8_t*)user_get_obj_addr(img->temp_payload, img->img_cur_pos, read_elem_cnt);
	
	internal_bmp_from_buf_str_memcpy_v2(buf, img, imgc, read_elem_cnt);
}
void VBDL_BMP_from_user_filesystem::fill_str_memclear(internal_draw_obj* img)
{
	if (img->temp_payload == NULL) return;
	
	user_fclose((USER_FILE**)&img->temp_payload);
}

#endif