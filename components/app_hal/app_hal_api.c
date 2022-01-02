#include "app_hal_api.h"




inline int32_t TICKS_OF_MS(int32_t ticks)
{
	return ticks / ((int32_t)portTICK_PERIOD_MS);
}

int32_t get_user_timer_value(TickType_t user_timer_buf)
{
	int32_t temp = xTaskGetTickCount();
	temp -= user_timer_buf;
	return temp;
}

TickType_t set_user_timer_value(int32_t new_value)
{
	TickType_t temp = (TickType_t)(-new_value);
	return xTaskGetTickCount() + temp;
}


/*
uint16_t swap_bytes_16(uint16_t var)
{
    uint8_t temp[2], *p_var=(uint8_t*)var;

    temp[0]=p_var[1];
    temp[1]=p_var[0];

    return *(uint16_t*)temp;
}

uint32_t swap_bytes_32(uint32_t var)
{
    uint8_t temp[4], *p_var=(uint8_t*)var;

    temp[0]=p_var[3];
    temp[1]=p_var[2];
    temp[2]=p_var[1];
    temp[3]=p_var[0];

    return *(uint32_t*)temp;
}
*/


/*
template <typename SW> void swap_values(SW& a, SW& b)
{
	SW temp = a;
	a = b;
	b = temp;
}
*/


uint16_t cast_to_module(int32_t num, uint16_t mod)
{
	if (mod == 0) return 0;
	num %= mod;
	if (num < 0) num += mod;
	return num;
}

uint16_t modulo_addition(int32_t num1, int32_t num2, uint16_t mod)
{
	return cast_to_module(num1 + num2, mod);
}




void safe_change_bits_u8(uint8_t *var, uint8_t bits, uint8_t mask)
{
	portMUX_TYPE xRegMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&xRegMutex);

	*var = ((*var)&(~mask)) | (bits&mask);

	portEXIT_CRITICAL(&xRegMutex);
}

void safe_change_bits_u16(uint16_t *var, uint16_t bits, uint16_t mask)
{
	portMUX_TYPE xRegMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&xRegMutex);

	*var = ((*var)&(~mask)) | (bits&mask);

	portEXIT_CRITICAL(&xRegMutex);
}

void safe_change_bits_u32(uint32_t *var, uint32_t bits, uint32_t mask)
{
	portMUX_TYPE xRegMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&xRegMutex);

	*var = ((*var)&(~mask)) | (bits&mask);

	portEXIT_CRITICAL(&xRegMutex);
}


int32_t round_value(float x)
{
	float add_part = (x > 0) ? (0.5) : (-0.5);
	return (x + add_part);
}


uint16_t PiecewiseLlinearApproximation0(uint16_t x, const PiecewiseLinearApproxPointTable* approx_table, uint8_t n_points)
{
	int32_t x0 = 0, x1, y0 = 0, y1;
	for (uint8_t i = 0; i < n_points; i++)
	{
		x1 = approx_table[i].x;
		y1 = approx_table[i].y;

		if (x < x1) return y0 + ((x - x0)*(y1 - y0)) / (x1 - x0);

		x0 = x1;
		y0 = y1;
	}

	return y0;
}



uint8_t int_log10(uint16_t var)
{
	for (uint8_t res = 0;; res++)
	{
		var /= 10;
		if (!var) return res;
	}
}


int32_t int32_pow(int32_t x, uint8_t y)
{
	int32_t res = 1;
	while (y--) res *= x;
	
	return res;
}


uint8_t sign_of_num_mod10(int32_t var, uint8_t n_sign)
{
	uint32_t temp = (var > 0) ? var : -var;
	uint32_t pow_10 = 1;
	
	while (n_sign--) pow_10 *= 10;
	
	return (temp / pow_10) % 10 ;
}




uint16_t find_index_of_mas(void* elem_ptr, void** in_mas, uint16_t mas_elem_count)
{
	uint16_t index = 0;
	for (; index < mas_elem_count; index++)
	{
		if (elem_ptr == in_mas[index]) break;
	}
	
	return index;
}



uint16_t cycle_through(uint16_t cur_value, uint16_t elem_count, int16_t inc)
{
	volatile int32_t temp = cur_value, inc32 = inc;
	temp = temp + inc32;
	
	while (temp >= elem_count) temp -= elem_count;
	while (temp < 0) temp += elem_count;
	
	return temp;
}


uint32_t FindHandle(void** list, void* data, size_t sizeof_data, uint32_t count)
{
	for (uint32_t i=0; i < count; i++, list += sizeof_data)
	{
		if (*list == data) return i;
	}

	return 0xFFFFFFFF;
}



const uint8_t random_numbers_regularity[256] = {
	 41,
	35,
	190,
	132,
	225,
	108,
	214,
	174,
	82,
	144,
	73,
	241,
	187,
	233,
	235,
	179,
	166,
	219,
	60,
	135,
	12,
	62,
	153,
	36,
	94,
	13,
	28,
	6,
	183,
	71,
	222,
	18,
	77,
	200,
	67,
	139,
	31,
	3,
	90,
	125,
	9,
	56,
	37,
	93,
	212,
	203,
	252,
	150,
	245,
	69,
	59,
	19,
	137,
	10,
	50,
	32,
	154,
	80,
	238,
	64,
	120,
	54,
	253,
	246,
	158,
	220,
	173,
	79,
	20,
	242,
	68,
	102,
	208,
	107,
	196,
	48,
	161,
	34,
	145,
	157,
	218,
	176,
	202,
	2,
	185,
	114,
	44,
	128,
	126,
	197,
	213,
	178,
	234,
	201,
	204,
	83,
	191,
	103,
	45,
	142,
	131,
	239,
	87,
	97,
	255,
	105,
	143,
	205,
	209,
	30,
	156,
	22,
	230,
	29,
	240,
	74,
	119,
	215,
	232,
	57,
	51,
	116,
	244,
	159,
	164,
	89,
	53,
	207,
	211,
	72,
	117,
	217,
	42,
	229,
	192,
	247,
	43,
	129,
	14,
	95,
	0,
	141,
	123,
	5,
	21,
	7,
	130,
	24,
	112,
	146,
	100,
	84,
	206,
	177,
	133,
	248,
	70,
	106,
	4,
	115,
	47,
	104,
	118,
	250,
	17,
	136,
	121,
	254,
	216,
	40,
	11,
	96,
	61,
	151,
	39,
	138,
	194,
	8,
	165,
	193,
	140,
	169,
	149,
	155,
	168,
	167,
	134,
	181,
	231,
	85,
	78,
	113,
	226,
	180,
	101,
	122,
	99,
	38,
	223,
	109,
	98,
	224,
	52,
	63,
	227,
	65,
	15,
	27,
	243,
	160,
	127,
	170,
	91,
	184,
	58,
	16,
	76,
	236,
	49,
	66,
	124,
	228,
	33,
	147,
	175,
	111,
	1,
	23,
	86,
	198,
	249,
	55,
	189,
	110,
	92,
	195,
	163,
	152,
	199,
	182,
	81,
	25,
	46,
	188,
	148,
	75,
	88,
	210,
	172,
	26,
	162,
	237,
	251,
	221,
	186,
	171
};

uint8_t random_regularity(uint8_t *index, uint8_t of_numbers_count)
{
	for (; *index < 255; (*index)++)
	{
		if (random_numbers_regularity[*index] < of_numbers_count) return random_numbers_regularity[*index];
	}
	
	return min1(random_numbers_regularity[255], of_numbers_count - 1);
}