#pragma once

#include "../display_datatypes.h"



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
#define BMP_CHROMAKEYED (1<<7)

//INTERNAL //8...16 bits
#define BMP_TYPE_MASK (7<<8)
#define BMP_RGB24 0
#define BMP_RGB16 (2<<8)
#define BMP_RGB8 (4<<8)
#define BMP_ARGB32 (1<<8)

#define BMP_COLORTYPE_MASK (3<<14)
#define BMP_INDEXED_8BIT (1<<14)
#define BMP_INDEXED_4BIT (2<<14)
//#define BMP_INDEXED_2BIT (3<<14)


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


#if defined(__cplusplus)

class VBDL_tImage : public draw_obj
{
public:
	VBDL_tImage(const tImage* img, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

protected:
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;
};

class VBDL_Embed_BMP : public draw_obj
{
public:
	VBDL_Embed_BMP(const char* bmpfile_start_pointer, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
	//VBDL_BMP(const char* filename, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

protected:
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;
};

class VBDL_BMP_from_file : public draw_obj
{
public:
	VBDL_BMP_from_file(const char* filename, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);

protected:
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;
};


#ifdef USER_FILESYSTEM
class VBDL_BMP_from_user_filesystem : public draw_obj
{
public:
	VBDL_BMP_from_user_filesystem(ufs_partition_t* partition, const char* filename, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align);
	~VBDL_BMP_from_user_filesystem();

protected:
	USER_FILE img_data;
	void* handle;
	void fill_str_init(std::vector <internal_draw_obj> &buf, rect mask, uint16_t layer_options) override;
	void fill_str_memcpy(uint8_t* buf, internal_draw_obj* img) override;
	void fill_str_memclear(internal_draw_obj* img) override;
    //~VBDL_Fill() override;
};
#endif


#endif