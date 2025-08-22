#pragma once


#include "app_hal_api.h"

#include "../drivers/user_lcd.h"
#include "display_options.h"
#include "display_init.h"
#include "display_led.h"

#include "display.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////


#define LAYER_OPTIONS_FIRST_LAYER (1<<0)
#define LAYER_OPTIONS_SINGLE_LAYER (1<<1)


///////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined(__cplusplus)

extern "C" {void init_display_orient();}

class VBDL_InternalPushLayer : public internal_draw_obj
{
    std::vector <internal_draw_obj> *out_buf;
    rect mask;
    void common_init(draw_obj* obj, std::unique_ptr<draw_obj> alloc_obj, rect mask, layer_options_t layer_options);
public:
    VBDL_InternalPushLayer(draw_obj* obj, rect mask, layer_options_t layer_options);
    VBDL_InternalPushLayer(std::unique_ptr<draw_obj> obj, rect mask, layer_options_t layer_options);
    ~VBDL_InternalPushLayer();

    bool is_overlap() {return buf_cnt_pix != 0;}
    void confirm(std::vector <internal_draw_obj> &buf) {out_buf = &buf;}

    static uint32_t calc_start_pos(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, uint8_t elem_per_pix);
    static uint32_t calc_start_pos_vertical_mirror(uint32_t img_bias_pix, int16_t y0, uint16_t img_width_elem, int16_t img_height, uint8_t elem_per_pix);

    static void update_area_on_screen(VBDisplay &Display, rect cur_pos, rect new_pos, uint32_t img_layer)
    {
        Display.update_area_on_screen(cur_pos, new_pos, img_layer);
    }
};

#endif








