#pragma once

#include "../draw.h"
//#include "ESP32-upng-master/upng.h"


typedef struct {
	const uint8_t *data;
	uint16_t width;
	uint16_t height;
	uint8_t dataSize;
} tImage;



//BMP_OPTIONS

//EXTERNAL //0...7 bits
#define BMP_VERTICAL_MIRROR (1<<2)
//#define BMP_HORIZONTAL_MIRROR (1<<3)
#define BMP_RGBA (1<<6)

//INTERNAL //8...16 bits
#define BMP_TYPE_MASK (7<<8)
#define BMP_RGB24 0
#define BMP_ARGB32 (1<<8)

#define BMP_SOURCE_MASK (7<<11)
#define BMP_BMPC 0
#define BMP_EMBED_BMP (1<<11)
#define BMP_FILE (2<<11)
#ifdef USER_FILESYSTEM
#define BMP_FROM_USER_FS (3<<11)
#endif


typedef struct
{
	uint32_t imageOffset;
	uint32_t imageWidth;
	uint32_t imageHeight;
	uint16_t imagePlanes;
	uint16_t imageBitsPerPixel;
	uint32_t imageCompression;
} bmpInfo;


bmpInfo read_bmp_header(uint8_t * header);




void bmp_str_init(internal_draw_obj* img);
void bmp_str_memcpy(uint8_t* buf, internal_draw_obj* img);
void bmp_str_memclear(internal_draw_obj* img);




draw_obj make_bmp_from_tImage(const tImage* img, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
draw_obj make_embed_bmp(const char* bmpfile_start_pointer, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
draw_obj make_file_bmp_from_file(const char* filename, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

#ifdef USER_FILESYSTEM
draw_obj make_bmp_from_user_filesystem(const char *file_name, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
#endif

