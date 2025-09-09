#pragma once

#include "display_options.h"
#include "display_init.h"
#include "display_led.h"
#include "draw.h"


#if ORIENTATIONS_COUNT == 4 || ORIENTATIONS_COUNT == 2
extern uint16_t _ScreenWidth;
extern uint16_t _ScreenHeight;
#endif


#include "display_macro.h"




#if defined(__cplusplus)


class VBDisplay
{
    friend class VBDL_InternalPushLayer;
    
public:

    draw_obj_list draw_buffer;
    size_t obj_count() {return draw_buffer.obj.size();}

    void common_draw(rect mask, uint32_t end_layer = -1);

    void redraw_all();
    void redraw_group(uint32_t first_layer, uint32_t end_layer);
    void redraw_objects_in_one_pass(std::vector <uint32_t> &obj_layers_list, rect* add_rect);
    void redraw_group_with_lower_layers_only(uint32_t first_ayer, uint32_t end_layer);

    static constexpr rect void_rect = draw_obj::void_rect;
    //static constexpr void* void_obj_data = (void*)&void_rect;
    static bool check_valid_obj(draw_obj* obj);
    static std::unique_ptr<draw_obj> make_void_obj();

    void clear_data();
    uint32_t set_or_update_obj(std::unique_ptr<draw_obj> obj, uint32_t* layer_num_handle = NULL, uint32_t desired_layer = -1, bool update_on_the_screen = 1);
    
    void pin_layer_handle_to_obj(uint32_t obj_layer, uint32_t* layer_num_handle);
    void change_obj_layer(uint32_t obj_layer, uint32_t new_layer);
    
    void redraw_obj(uint32_t img_layer);
    void delete_obj(uint32_t img_layer);
    void hide_obj(uint32_t img_layer);
    void update_obj(std::unique_ptr<draw_obj> new_obj, uint32_t img_layer);
    
    void move_obj_to_new_xy(draw_obj* obj, int16_t new_x, int16_t new_y, uint8_t align);
    void move_obj_dxdy(draw_obj* obj, int16_t dx, int16_t dy);
    void move_obj_to_new_xy(uint32_t img_layer, int16_t new_x, int16_t new_y, uint8_t align, bool update_on_the_screen = 1);
    void move_obj_dxdy(uint32_t img_layer, int16_t dx, int16_t dy, bool update_on_the_screen = 1);
    void align_obj_group(std::vector <uint32_t> &obj_layers_list, int16_t new_x, int16_t new_y, uint8_t align);
    void align_obj_group(std::vector <draw_obj*> &obj_list, int16_t new_x, int16_t new_y, uint8_t align);
    void align_obj_group(std::vector <std::unique_ptr<draw_obj>> &obj_list, int16_t new_x, int16_t new_y, uint8_t align);
    static uint16_t get_obj_width(draw_obj* obj);
    static uint16_t get_obj_height(draw_obj* obj);

    void screen_buf_insert_obj(std::unique_ptr<draw_obj> obj, uint32_t* layer_num_handle, uint32_t layer = (uint32_t)(-1));
    draw_obj* screen_buf_get_obj(uint32_t layer);
    void screen_buf_delete_obj(uint32_t layer);
    void screen_buf_update_obj(std::unique_ptr<draw_obj> obj, uint32_t layer);

    static uint8_t* alloc_additional_str_buffer_to_draw();
    static uint32_t invert_color(uint32_t color);
    static uint32_t invert_brightness(uint32_t color);
    static rect get_rect(int16_t x0, int16_t y0, uint16_t width, uint16_t height, uint8_t align);
    static bool crop_rect_within_screen(rect* r);
    static bool check_valid_rect(rect r);
    static bool check_valid_rect(rect* r);
    static bool check_equal_rects(rect r1, rect r2);
    static bool check_equal_rects(rect* r1, rect* r2);
    static bool check_horizontal_overlap(rect img_pos, rect mask);
    static bool check_vertical_overlap(rect img_pos, rect mask);
    static rect min_rect(rect r1, rect r2);
    static rect max_rect(rect r1, rect r2);
    static rect max_rect_no_valid_check(rect r1, rect r2);
    static rect min_rect_no_valid_check(rect r1, rect r2);
    static void move_rect_to_new_xy(rect *r, int16_t new_x, int16_t new_y);
    static void move_rect_dxdy(rect *r, int16_t dx, int16_t dy);
    static int16_t calc_hor_offset(int16_t width, uint8_t align);
    static int16_t calc_ver_offset(int16_t height, uint8_t align);

private:

#ifdef ENABLE_FRAGMEMT_DRAWING
    void internal_common_draw(rect mask, uint32_t end_layer);
#endif

    void update_area_on_screen(rect cur_pos, rect new_pos, uint32_t img_layer);
    static void lcdInternalSetRect(rect r);

    static uint8_t* string_alloc_buffer;
};


#endif
	




