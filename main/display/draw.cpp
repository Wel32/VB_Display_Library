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








void draw_string(draw_obj_list draw_buffer, uint8_t layer, draw_obj_list img_str)
{
	if (layer >= draw_buffer.elem_cnt) return;
	
	rect old_rect = draw_buffer.obj[layer].pos;
	uint8_t init = 0;
	int16_t x = 0;

	rect new_rect;

	for (uint8_t i = 0; i < img_str.elem_cnt; i++)
	{
		if (!check_rect_direction(&img_str.obj[i].pos)) continue;

		draw_buffer.obj[layer].pos = old_rect;

		if (!init)
		{
			init = 1;

			new_rect = img_str.obj[i].pos;
		}
		else
		{
			if (img_str.obj[i].pos.x0 < x)
			{
				delete_img(draw_buffer, layer);
				draw_buffer.obj[layer].pos = new_rect;
				delete_img(draw_buffer, layer);
				return;
			}

			clamp_min(draw_buffer.obj[layer].pos.x0, x);

			new_rect = internal_max_rect(new_rect, img_str.obj[i].pos);
		}

		clamp_max(draw_buffer.obj[layer].pos.x1, img_str.obj[i].pos.x1);

		_update_img(draw_buffer, layer, &img_str.obj[i]);

		x = img_str.obj[i].pos.x1 + 1;
	}

	draw_buffer.obj[layer].pos = old_rect;
	clamp_min(draw_buffer.obj[layer].pos.x0, x);

	delete_img(draw_buffer, layer);

	draw_buffer.obj[layer].pos = new_rect;
	draw_buffer.obj[layer].obj_type = TVOID;
}


void draw_strings(draw_obj_list draw_buffer, uint8_t layer, draw_obj_list* img_str, uint8_t str_cnt)
{
	if (layer >= draw_buffer.elem_cnt) return;
	
	rect old_rect = draw_buffer.obj[layer].pos;
	rect common_new_rect, new_rect;

	uint8_t init = 0;
	int16_t y = -1;

	for (uint8_t i = 0; i < str_cnt; i++)
	{
		draw_buffer.obj[layer].pos = old_rect;
		new_rect = common_rect(img_str[i].obj, img_str[i].elem_cnt);

		if (!check_rect_direction(&new_rect)) continue;

		if (init) clamp_max(draw_buffer.obj[layer].pos.y1, y);
		clamp_min(draw_buffer.obj[layer].pos.y0, new_rect.y0);

		draw_string(draw_buffer, layer, img_str[i]);

		y = new_rect.y0 - 1;

		if (!init)
		{
			init = 1;

			common_new_rect = new_rect;
		}
		else common_new_rect = internal_max_rect(common_new_rect, new_rect);
	}

	draw_buffer.obj[layer].pos = old_rect;
	clamp_max(draw_buffer.obj[layer].pos.y1, y);

	delete_img(draw_buffer, layer);

	draw_buffer.obj[layer].pos = common_new_rect;
	draw_buffer.obj[layer].obj_type = TVOID;
}