#pragma once

#include "../draw.h"
#include "gamma.h"
#include "draw_bmp.h"



typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
} tSprite;



void spr_str_init(internal_draw_obj* img);
void spr_str_memcpy(uint8_t* buf, internal_draw_obj* img);
void spr_str_memclear(internal_draw_obj* img);




draw_obj make_sprite(const tSprite* spr, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);