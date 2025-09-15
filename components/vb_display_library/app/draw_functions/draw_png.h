#pragma once


#ifdef USER_FILESYSTEM

#include "user_filesystem.h"

#if defined(__cplusplus)

class VBDL_uPNG : public VBDL_tImage
{
public:
	VBDL_uPNG(const ufs_partition_t* partition, const char *file_name, int16_t x, int16_t y, uint32_t color, uint8_t options, uint8_t align, bool lcd_off_by_decoding = 0);
    ~VBDL_uPNG();

	bool isOK() {return png_buf != NULL && handle != NULL;}

protected:

	void* png_buf;
};

#endif

#endif
