#include "user_menu_screens.h"

//#define ORIENTATION_DIR(por, al) por

draw_obj_list Display_data;
draw_obj_list* Display = &Display_data;

void* Screen1::Select()
{
	clear_screen_data(Display);

	set_or_update_obj(Display, make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN), NULL, 1, 1);

	vTaskDelay(2000 / portTICK_PERIOD_MS);


	tFont_text caption_text;

	caption_text.text = L"HELLO, WORLD!!!";
	caption_text.layer = -1;
	caption_text.font = arial_25;
	caption_text.align = CENTER_ALIGN | CENTER_ALIGN;
	//caption_text.char_space.push_back(1);
	caption_text.string_space = 80;

	int16_t text_x0 = ScreenWidth/2, text_y0 = ScreenHeight/2, text_place_width = ScreenWidth;

	if (PortraitOrientation)
	{
		text_y0 +=40;
		text_place_width = 200;
	}

	tFont_text_line_autobreake(caption_text, text_place_width, 0, 2, 2);
	set_tFont_text(Display, NULL, caption_text, text_x0, text_y0, 0xFF0000, 0);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen2::Select;
}

void* Screen2::Select()
{
	clear_screen_data(Display);

	set_or_update_obj(Display, make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN), NULL, 1, 1);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	tFont_text caption_text;

	caption_text.text = L"This is a new multilayer LCD library";
	caption_text.layer = -1;
	caption_text.font = Type36;
	caption_text.align = CENTER_ALIGN | CENTER_ALIGN;
	caption_text.string_space = 80;

	int16_t text_place_width = ScreenWidth - 100;
	tFont_text_line_autobreake(caption_text, text_place_width, -3, 2, 10);

	uint8_t str_count = 1;
	for (uint8_t i = 0; i<caption_text.text.size(); i++) if (caption_text.text[i] == L'\n') str_count++;

	int16_t start_y = (ScreenHeight + caption_text.string_space * (str_count - 1))/2;

	set_tFont_text(Display, NULL, caption_text, ScreenWidth / 2, start_y, 0xFFFF00, DRAW_INV_TRANSPARENCY);

	vTaskDelay(5000 / portTICK_PERIOD_MS);

	int16_t text_size_y = Display->obj[caption_text.layer].pos.y1 - Display->obj[caption_text.layer].pos.y0;
	int16_t text_y1 = Display->obj[caption_text.layer].pos.y1;

	uint32_t rect_layer = -1;
	set_or_update_obj(Display,
		make_fill(Display->obj[caption_text.layer].pos.x0,
			Display->obj[caption_text.layer].pos.y0 - text_size_y,
			Display->obj[caption_text.layer].pos.x1,
			Display->obj[caption_text.layer].pos.y1 - text_size_y,
			0xFF000000, 0),
		&rect_layer, 1, 1); //100% transparency

	

	screen_buf_delete_obj(Display, caption_text.layer); //delete text from buffer but not from display yet

	do
	{
		common_draw(Display, Display->obj[rect_layer].pos, -1);

		move_obj_dxdy(Display, rect_layer, 0, 2);

		vTaskDelay(20 / portTICK_PERIOD_MS);

	} while (Display->obj[rect_layer].pos.y1 < text_y1);


	vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen3::Select;
}




void* Screen3::Select()
{
	clear_screen_data(Display);

	set_or_update_obj(Display, make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN), NULL, 1, 1);

	vTaskDelay(5000 / portTICK_PERIOD_MS);
	
	uint32_t text_color[3] = { 0xFF00, 0xFFFF00, 0xFF0000 };
	uint8_t text_color_i = 0;

	tFont_text caption_text;

	caption_text.text = L"LET'S SOME SNOW!";
	caption_text.layer = -1;
	caption_text.font = arial_25;
	caption_text.align = CENTER_ALIGN | CENTER_ALIGN;
	caption_text.string_space = 80;
	
	int16_t text_place_width;

	if (PortraitOrientation) text_place_width = 100;
	else text_place_width = ScreenWidth - 200;
	
	tFont_text_line_autobreake(caption_text, text_place_width, -3, 2, 10);

	uint8_t str_count = 1;
	for (uint8_t i = 0; i<caption_text.text.size(); i++) if (caption_text.text[i] == L'\n') str_count++;

	int16_t text_start_y = (ScreenHeight + caption_text.string_space * (str_count - 1))/2;

	draw_obj text_obj = make_tFont_text(Display, caption_text, ScreenWidth / 2, text_start_y, text_color[text_color_i], 0);
	set_or_update_obj(Display, text_obj, &caption_text.layer, 1, 1);
	

	uint32_t sprites_start_layer = -1;
	uint8_t transparency = 60;
	int16_t y0 = ScreenHeight;
	int16_t dy[5] = { 40, 20, 60, 0, 40 };
	int16_t common_dx = ScreenWidth / 5;
	int16_t common_dy = -ScreenHeight / 5;

	for (uint8_t j = 0; j < 5; j++)
	{
		int16_t x0 = (j & 1) ? (common_dx / 4) : (3*common_dx / 4);

		for (uint8_t i = 0; i < 5; i++)
		{
			uint32_t* sprite0_layer_ptr = (sprites_start_layer == -1) ? &sprites_start_layer : NULL;
			set_or_update_obj(Display, make_sprite(&snow_spr, x0, y0 + dy[i], 0xFFFFFF | ((uint32_t)transparency << 24), 0, CENTER_ALIGN | CENTER_ALIGN), sprite0_layer_ptr, 1, 1);

			x0 += common_dx;
		}

		y0 += common_dy;
	}





	uint8_t dx[25];
	for (uint8_t i = 0; i < 25;)
	{
		dx[i++] = 107;
		dx[i++] = 41;
		dx[i++] = 00;
		dx[i++] = 63;
		dx[i++] = 204;
	}

	for (uint16_t j = 0; j < 2000; j++)
	{
		if (!(j % 200))
		{
			text_color_i = modulo_addition(text_color_i, +1, sizeof(text_color)/sizeof(uint32_t));

			set_or_update_obj(Display, make_tFont_text(Display, caption_text, ScreenWidth / 2, text_start_y, text_color[text_color_i], 0), &caption_text.layer, 1, 1);
		}
		
		vTaskDelay(1);
		for (uint8_t i = 0; i < 25; i++)
		{
			int16_t local_dx = (dx[i] % 4) ? 0 : 1;
			if ((dx[i] % 128) > 63) local_dx = -local_dx;

			dx[i]++;

			move_obj_dxdy(Display, i + sprites_start_layer, local_dx, -1);

			if (Display->obj[i + sprites_start_layer].pos.y1 < 0) move_obj_dxdy(Display, i + sprites_start_layer, 0, ScreenHeight);
		}
	}

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	lcdChangeOrient(+1);
	//lcdChangeOrient();


	return (into_menu) Screen1::Select;
}


