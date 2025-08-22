#pragma once

#include "../display_datatypes.h"


#if defined(__cplusplus)

class VBDL_Fill : public draw_obj
{
public:
    VBDL_Fill(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color, uint8_t options);
	VBDL_Fill(rect pos, uint32_t color, uint8_t options);

protected:
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;
};

#endif