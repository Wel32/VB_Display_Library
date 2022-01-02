#pragma once
#include "app_hal_api.h"






class UserMenuTimer
{
	int32_t TickBuffer;
	
	void Set(int32_t init_ticks) {TickBuffer = (int32_t)xTaskGetTickCount() - init_ticks; }
	
	bool Tick(int32_t cnt_value_ticks, bool reset)
	{
		int32_t current_cnt = xTaskGetTickCount();
		
		volatile int32_t dif = current_cnt - TickBuffer - cnt_value_ticks;
		
		if (dif >= 0)
		{
			if (dif > MAX_COUNTED_TIME)
			{
				TickBuffer += dif - MAX_COUNTED_TIME;
				return 0;
			}
			
			if (reset) TickBuffer = current_cnt;
			
			return 1;
		}
		return 0;
	}
	
public:
	typedef enum{ MAX_COUNTED_TIME = 0xFFFFFFF } max_ct;
	UserMenuTimer() {Set(0); }
	UserMenuTimer(int32_t init_ms) { Set_ms(init_ms); }
	
	int32_t GetCurrentTime_ms(){return (xTaskGetTickCount() - TickBuffer) * ((int32_t)portTICK_PERIOD_MS);}
	inline void Set_ms(int32_t init_ms) {Set(TICKS_OF_MS(init_ms)); }
	inline bool Tick_ms(int32_t cnt_value_ms, bool reset) {return Tick(TICKS_OF_MS(cnt_value_ms), reset); }
};

