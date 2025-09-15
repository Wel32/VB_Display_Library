#pragma once

#include "../display_datatypes.h"
#include "gamma.h"
#include "draw_bmp.h"


typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
} tSprite;


#if defined(__cplusplus)

class VBDL_Sprite : public draw_obj
{
public:
	VBDL_Sprite(const tSprite* spr, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
	VBDL_Sprite(const uint8_t* spr, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

protected:
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;

private:
	uint8_t conv_data_color(uint8_t data_color, uint8_t options);
};

#endif