#include "app_hal_pwm.h"
#include "system_include.h"


app_err_t hal_pwm_channel_config(const hal_pwm_channel_t* pwm_channel_conf)
{
    ledc_channel_config_t lcc = pwm_channel_conf->ledc_channel_conf;
    lcc.duty = 0;

    esp_err_t res = ledc_channel_config(&lcc);
    if (res == ESP_OK)
    {
        ledc_fade_func_install(0);

        ledc_stop(lcc.speed_mode, lcc.channel, pwm_channel_conf->gpio_idle_level);
        ledc_set_duty(pwm_channel_conf->ledc_channel_conf.speed_mode, pwm_channel_conf->ledc_channel_conf.channel, pwm_channel_conf->ledc_channel_conf.duty);
    }

    return res;
}

app_err_t hal_pwm_stop(const hal_pwm_channel_t* pwm_channel)
{
    return ledc_stop(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.channel, pwm_channel->gpio_idle_level);
}

app_err_t hal_pwm_stop_with_idle_level(const hal_pwm_channel_t* pwm_channel, uint32_t idle_level)
{
    return ledc_stop(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.channel, idle_level);
}

app_err_t hal_pwm_start(const hal_pwm_channel_t* pwm_channel)
{
    return ledc_update_duty(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.channel);
}

app_err_t hal_pwm_set_freq(const hal_pwm_channel_t* pwm_channel, uint32_t freq_hz)
{
    return ledc_set_freq(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.timer_sel, freq_hz);
}

app_err_t hal_pwm_set_duty(const hal_pwm_channel_t* pwm_channel, uint32_t duty)
{
    esp_err_t res = ledc_set_duty(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.channel, duty);
    if (res == ESP_OK) ledc_update_duty(pwm_channel->ledc_channel_conf.speed_mode, pwm_channel->ledc_channel_conf.channel);

    return res;
}

app_err_t hal_pwm_restart(const hal_pwm_channel_t* pwm_channel)
{
    return hal_pwm_start(pwm_channel);
}
