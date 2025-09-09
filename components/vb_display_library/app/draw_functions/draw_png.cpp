#include "../display_core.h"
#include "../draw.h"
#include "draw_png.h"
#include "draw_bmp.h"


#ifdef USER_FILESYSTEM


#include "ESP32-upng-master/upng.h"


VBDL_uPNG::VBDL_uPNG(const ufs_partition_t* partition, const char *file_name, int16_t _x, int16_t _y, uint32_t _color, uint8_t _options, uint8_t _align, bool lcd_off_by_decoding) :
    VBDL_tImage((const tImage*)NULL, 0, 0, 0, 0, 0)
{
    png_buf = NULL;
    bool png_decode_ok = 0;
	USER_FILE outfile;
	
	if (!ufs_find(partition, file_name, &outfile, 0))
	{
		ESP_LOGI("PNG", "file not found: %s", file_name);
		return;
	}

    png_buf = malloc(sizeof(decoded_bmp_t));
    upng_t* upng = upng_new_from_bytes((const unsigned char*)outfile.addr, outfile.size);
		
	if (upng != NULL)
	{
        if (lcd_off_by_decoding) lcdOff();

        upng_decode_from_user_fs(upng, user_get_obj_addr, UFS_MAX_PART_SIZE);
		if (upng_get_error(upng) == UPNG_EOK) png_decode_ok = 1;
	}

    if (png_decode_ok)
    {
        uint16_t width = upng_get_width(upng);
		uint16_t height = upng_get_height(upng);
		uint8_t depth = upng_get_bpp(upng);

        ((decoded_bmp_t*)png_buf)->codec_data = upng;
		((decoded_bmp_t*)png_buf)->decoded_data = (const uint8_t*) upng_get_buffer(upng);

		tImage buf_image = {((decoded_bmp_t*)png_buf)->decoded_data, width, height, depth};
        if (depth == 32) _options |= BMP_RGBA;

        VBDL_tImage bmp_output_image(&buf_image, _x, _y, _color, _options^BMP_VERTICAL_MIRROR, _align);

        VBDL_tImage* this_obj_bmp = this;
        *this_obj_bmp = bmp_output_image;
    }
    else
    {
        free(png_buf);
        ESP_LOGI("PNG", "file decode error: %s", file_name);
    }
}


VBDL_uPNG::~VBDL_uPNG()
{
    if (png_buf != NULL)
    {
        decoded_bmp_t* decoded_buf = (decoded_bmp_t*)png_buf;
        upng_t* upng = (upng_t*)(decoded_buf->codec_data);
    
        upng_free(upng);
        free(png_buf);
    }
}

#endif