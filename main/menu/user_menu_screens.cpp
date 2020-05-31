
#include "user_menu_screens.h"

//#define PortraitOrientation 1
//#define ORIENTATION_DIR(por, al) por

void* Screen1::Select()
{
	NEW_SCREEN(screen, screen_data, 10);
	
	screen_data[0] = make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	redraw_all(screen);

	vTaskDelay(2000 / portTICK_PERIOD_MS);


	if (PortraitOrientation)
	{
		wstring str1 = L"HELLO,";
		wstring str2 = L"WORLD!!!";

		const wchar_t *text[2] = { str1.data(), str2.data() };

		draw_tFont_strings(screen, 1, arial_25, 1, 80, text, 2, 160, 280, 0xFF0000, 0, CENTER_ALIGN | CENTER_ALIGN);
	}
	else
	{
		wstring str = L"HELLO, WORLD!!!";

		draw_tFont_string(screen, 1, arial_25, 1, str.data(), 240, 160, 0xFF0000, 0, CENTER_ALIGN | CENTER_ALIGN);
	}
	
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen2::Select;
}


void* Screen4::Select()
{
	NEW_SCREEN(screen, screen_data, 50);
	
	
	for (uint8_t i = 0; i < 4; i++)
	{
		screen_data[i] = make_bmp_from_tImage(&picture1, 240*(i / 2), 240*(i % 2), 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	}
	
	redraw_all(screen);

	while(1) vTaskDelay(2000 / portTICK_PERIOD_MS);

	return (into_menu) Screen2::Select;
}

void* Screen2::Select()
{
	NEW_SCREEN(screen, screen_data, 2);
	
	screen_data[0] = make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	redraw_all(screen);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	wstring str1 = L"This is a new";
	wstring str2 = L"multilayer";
	wstring str3 = L"LCD library";
	
	uint8_t str_cnt;
	int16_t start_y = ScreenHeight / 2;
	int16_t str_dy = 80;
	
	if (PortraitOrientation)
	{
		str_cnt = 3;
		start_y += str_dy;
	}
	else
	{
		str_cnt = 2;
		start_y += str_dy/2;
		
		str2 += L" ";
		str2 += str3;
	}

	const wchar_t *text[3] = { str1.data(), str2.data(), str3.data()  };

	tFont text_font = Type36;
	draw_fitted_tFont_strings(screen, 1, text_font, -3, 2, ScreenWidth - 80, str_dy, text, str_cnt, ScreenWidth / 2, start_y, 0xFFFF00, DRAW_INV_TRANSPARENCY, CENTER_ALIGN | CENTER_ALIGN);
	
	vTaskDelay(5000 / portTICK_PERIOD_MS);

	rect clear_text_mask = screen_data[1].pos;
	move_rect_dxdy(&clear_text_mask, 0, screen_data[1].pos.y0 - screen_data[1].pos.y1);
	
	screen_data[1] = make_void_obj();

	do
	{
		common_draw(screen, clear_text_mask);
		move_rect_dxdy(&clear_text_mask, 0, 2);

		vTaskDelay(20 / portTICK_PERIOD_MS);

	} while (clear_text_mask.y1 < screen_data[1].pos.y1);


	vTaskDelay(2000 / portTICK_PERIOD_MS);


	return (into_menu) Screen3::Select;
}




void* Screen3::Select()
{
	NEW_SCREEN(screen, screen_data, 60);
	
	screen_data[0] = make_bmp_from_tImage(&picture1, 0, 0, 0, 0, LEFT_ALIGN | BOTTOM_ALIGN);
	redraw_all(screen);

	uint8_t text_start_layer = 1;

	vTaskDelay(5000 / portTICK_PERIOD_MS);
	
	wstring str1;
	wstring str2;
	wstring str3;
	
	uint32_t str_color[3] = { 0xFF00, 0xFFFF00, 0xFF0000 };
	
	if (PortraitOrientation)
	{
		str1 = L"LET'S";
		str2 = L"SOME";
		str3 = L"SNOW!";
	}
	else
	{
		str1 = L"LET'S";
		str2 = L"SOME SNOW!";
	}
	

	uint8_t sprites_start_layer = text_start_layer+30;

	uint8_t transparency = 60;
	screen_data[sprites_start_layer] = make_sprite(&snow_spr, 0, 0, 0xFFFFFF | ((uint32_t)transparency << 24), 0, CENTER_ALIGN | CENTER_ALIGN);
	
	for (uint8_t i = sprites_start_layer + 1; i < sprites_start_layer + 25; i++) screen_data[i] = screen_data[sprites_start_layer];

	int16_t y0 = ScreenHeight;
	int16_t dy[5] = { 40, 20, 60, 0, 40 };
	int16_t common_dx = ScreenWidth / 5;
	int16_t common_dy = -ScreenHeight / 5;

	for (uint8_t j = 0; j < 5; j++)
	{
		int16_t x0 = (j & 1) ? (common_dx / 4) : (3*common_dx / 4);

		for (uint8_t i = 0; i < 5; i++)
		{
			move_rect_to_new_xy(&screen_data[i + j * 5 + sprites_start_layer].pos, x0, y0 + dy[i]);
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
			int16_t str_dy = 80;
			int16_t y0 = (ScreenHeight / 2) + str_dy*((str3.length()) ? 2 : 1) / 2;
			
			draw_tFont_string_separate_sprites(screen, text_start_layer, 6, arial_25, 1, str1.data(), ScreenWidth / 2, y0, str_color[0], 0, CENTER_ALIGN | CENTER_ALIGN);
			y0 -= str_dy;
			draw_tFont_string_separate_sprites(screen, text_start_layer + 6, 10, arial_25, 1, str2.data(), ScreenWidth / 2, y0, str_color[1], 0, CENTER_ALIGN | CENTER_ALIGN);
			y0 -= str_dy;
			if (str3.length()) draw_tFont_string_separate_sprites(screen, text_start_layer + 16, 10, arial_25, 1, str3.data(), ScreenWidth / 2, y0, str_color[2], 0, CENTER_ALIGN | CENTER_ALIGN);
			
			uint32_t temp = str_color[2];
			str_color[2] = str_color[1];
			str_color[1] = str_color[0];
			str_color[0] = temp;
			
			vTaskDelay(1);
		}
		
		for (uint8_t i = 0; i < 25; i++)
		{
			int16_t local_dx = (dx[i] % 4) ? 0 : 1;
			if ((dx[i] % 128) > 63) local_dx = -local_dx;

			dx[i]++;

			move_img_dxdy(screen, i + sprites_start_layer, local_dx, -1);

			if (screen_data[i + sprites_start_layer].pos.y1 < 0) move_rect_dxdy(&screen_data[i + sprites_start_layer].pos, 0, ScreenHeight);
		}
	}

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	lcdChangeOrient(+1);
	//lcdChangeOrient();

	return (into_menu) Screen1::Select;
}


