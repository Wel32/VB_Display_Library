#include "user_menu_screens.h"

//#define ORIENTATION_DIR(por, al) por
VBDisplay Display;

void* Screen1::Select()
{
	Display.clear_data();

	Display.set_or_update_obj(std::make_unique<VBDL_tImage>(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN));

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	tFontText caption_text(arial_25, CENTER_ALIGN | CENTER_ALIGN, 0, 80);
	caption_text.text = L"HELLO, WORLD!!!";

	int16_t text_x0 = ScreenWidth/2, text_y0 = ScreenHeight/2, text_place_width = ScreenWidth;

	if (PortraitOrientation)
	{
		text_y0 +=40;
		text_place_width = 200;
	}

	VBDL_tFontText::text_autowrap(caption_text, text_place_width, 0, 2, 2);
	
	Display.set_or_update_obj(std::make_unique<VBDL_tFontText>(caption_text, text_x0, text_y0, 0xFF0000, 0));

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen2::Select;
}

void* Screen2::Select()
{
	Display.clear_data();

	Display.set_or_update_obj(std::make_unique<VBDL_tImage>(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN));

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	tFontText caption_text(Type36, CENTER_ALIGN | CENTER_ALIGN, 0, 80);
	caption_text.text = L"This is a lightweight multilayer LCD library";

	uint32_t caption_text_layer = -1;
	int16_t text_place_width = ScreenWidth - 100;
	VBDL_tFontText::text_autowrap(caption_text, text_place_width, -3, 2, 10);

	uint8_t str_count = 1;
	for (uint8_t i = 0; i<caption_text.text.size(); i++) if (caption_text.text[i] == L'\n') str_count++;

	int16_t start_y = (ScreenHeight + caption_text.string_space * (str_count - 1))/2;

	VBDL_tFontText::set_or_update_text(Display, &caption_text_layer, caption_text, ScreenWidth / 2, start_y, 0xFFFF00, DRAW_INV_TRANSPARENCY);

	vTaskDelay(5000 / portTICK_PERIOD_MS);

	rect caption_text_pos = Display.screen_buf_get_obj(caption_text_layer)->pos;
	int16_t text_size_y = caption_text_pos.y1 - caption_text_pos.y0;
	int16_t text_y1 = caption_text_pos.y1;

	uint32_t rect_layer = -1;

	Display.set_or_update_obj(
		std::make_unique<VBDL_Fill>(caption_text_pos.x0, caption_text_pos.y0 - text_size_y, caption_text_pos.x1, caption_text_pos.y1 - text_size_y, 0xFF000000, 0),
		&rect_layer); //100% transparency

	Display.screen_buf_delete_obj(caption_text_layer); //delete text from buffer but not from display yet

	do
	{
		Display.common_draw(Display.screen_buf_get_obj(rect_layer)->pos, -1);
		Display.move_obj_dxdy(rect_layer, 0, 2);

		vTaskDelay(20 / portTICK_PERIOD_MS);

	} while (Display.screen_buf_get_obj(rect_layer)->pos.y1 < text_y1);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen3::Select;
}




void* Screen3::Select()
{
	Display.clear_data();

	Display.set_or_update_obj(std::make_unique<VBDL_tImage>(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN));

	vTaskDelay(5000 / portTICK_PERIOD_MS);
	
	uint32_t text_color[3] = { 0xFF00, 0xFFFF00, 0xFF0000 };
	uint8_t text_color_i = 0;

	tFontText caption_text(arial_25, CENTER_ALIGN | CENTER_ALIGN, 0, 80);
	caption_text.text = L"LET'S SOME SNOW!";
	
	uint32_t caption_text_layer = -1;
	int16_t text_place_width;

	if (PortraitOrientation) text_place_width = 100;
	else text_place_width = ScreenWidth - 200;
	
	VBDL_tFontText::text_autowrap(caption_text, text_place_width, -3, 2, 10);

	uint8_t str_count = 1;
	for (uint8_t i = 0; i<caption_text.text.size(); i++) if (caption_text.text[i] == L'\n') str_count++;

	int16_t text_start_y = (ScreenHeight + caption_text.string_space * (str_count - 1))/2;

	Display.set_or_update_obj(std::make_unique<VBDL_tFontText>(caption_text, ScreenWidth / 2, text_start_y, text_color[text_color_i], 0), &caption_text_layer);


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
			Display.set_or_update_obj(
				std::make_unique<VBDL_Sprite>(&snow_spr, x0, y0 + dy[i], 0xFFFFFF | ((uint32_t)transparency << 24), 0, CENTER_ALIGN | CENTER_ALIGN),
				(sprites_start_layer == -1) ? &sprites_start_layer : NULL);

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

			VBDL_tFontText::set_or_update_text(Display, &caption_text_layer, caption_text, ScreenWidth / 2, text_start_y, text_color[text_color_i], 0);
		}
		
		vTaskDelay(1);
		for (uint8_t i = 0; i < 25; i++)
		{
			int16_t local_dx = (dx[i] % 4) ? 0 : 1;
			if ((dx[i] % 128) > 63) local_dx = -local_dx;

			dx[i]++;

			Display.move_obj_dxdy(i + sprites_start_layer, local_dx, -1);
			if (Display.screen_buf_get_obj(i + sprites_start_layer)->pos.y1 < 0) Display.move_obj_dxdy(i + sprites_start_layer, 0, ScreenHeight);
		}
	}

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	lcdChangeOrient(+1);
	//lcdChangeOrient();


	return (into_menu) Screen1::Select;
}


