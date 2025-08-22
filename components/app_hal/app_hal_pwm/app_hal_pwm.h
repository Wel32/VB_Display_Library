#pragma once
#include "app_hal_api.h"




#define hal_pwm_timer_config(pwm_timer) ledc_timer_config(pwm_timer)




#if defined(__cplusplus)
extern "C" {
#endif

typedef ledc_timer_config_t hal_pwm_timer_config_t;

typedef struct
{
    ledc_channel_config_t ledc_channel_conf;
    uint32_t gpio_idle_level;
}
hal_pwm_channel_config_t;


app_err_t hal_pwm_channel_config(const hal_pwm_channel_config_t* pwm_channel_conf);
app_err_t hal_pwm_stop(const hal_pwm_channel_config_t* pwm_channel);
app_err_t hal_pwm_stop_with_idle_level(const hal_pwm_channel_config_t* pwm_channel, uint32_t idle_level);
app_err_t hal_pwm_start(const hal_pwm_channel_config_t* pwm_channel);
app_err_t hal_pwm_restart(const hal_pwm_channel_config_t* pwm_channel);
app_err_t hal_pwm_set_freq(const hal_pwm_channel_config_t* pwm_channel, uint32_t freq_hz);
app_err_t hal_pwm_set_duty(const hal_pwm_channel_config_t* pwm_channel, uint32_t duty);

#if defined(__cplusplus)
}
#endif
