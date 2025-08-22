#include "pcb_config.h"



const hal_pwm_timer_config_t lcd_bl_pwm_timer_data = 
{
    LEDC_HIGH_SPEED_MODE,                /*!< LEDC speed speed_mode, high-speed mode or low-speed mode */
    LEDC_TIMER_8_BIT,  /*!< LEDC channel duty resolution */
    LEDC_TIMER_1,               /*!< The timer source of channel (0 - 3) */
    2000,                      /*!< LEDC timer frequency (Hz) */
    LEDC_AUTO_CLK                /*!< Configure LEDC source clock.
                                                For low speed channels and high speed channels, you can specify the source clock using LEDC_USE_REF_TICK, LEDC_USE_APB_CLK or LEDC_AUTO_CLK.
                                                For low speed channels, you can also specify the source clock using LEDC_USE_RTC8M_CLK, in this case, all low speed channel's source clock must be RTC8M_CLK*/
};
const hal_pwm_timer_config_t* lcd_bl_pwm_timer = &lcd_bl_pwm_timer_data;

const hal_pwm_channel_config_t lcd_bl_pwm_channel_data = 
{
    {GPIO_NUM_16,                  /*!< the LEDC output gpio_num, if you want to use gpio16, gpio_num = 16 */
    LEDC_HIGH_SPEED_MODE,         /*!< LEDC speed speed_mode, high-speed mode or low-speed mode */
    LEDC_CHANNEL_2,         /*!< LEDC channel (0 - 7) */
    LEDC_INTR_FADE_END,     /*!< configure interrupt, Fade interrupt enable  or Fade interrupt disable */
    LEDC_TIMER_1,         /*!< Select the timer source of channel (0 - 3) */
    0,                  /*!< LEDC channel duty, the range of duty setting is [0, (2**duty_resolution)] */
    0},                     /*!< LEDC channel hpoint value, the max value is 0xfffff */
    0                     /*idle gpio state */
};
const hal_pwm_channel_config_t* lcd_bl_pwm_channel = &lcd_bl_pwm_channel_data;




//const hal_gpio_num_t lcd_miso_gpio = GPIO_NUM_12;
const hal_gpio_num_t lcd_mosi_gpio = GPIO_NUM_13;
const hal_gpio_num_t lcd_clk_gpio = GPIO_NUM_14;
//const hal_gpio_num_t lcd_cs_gpio = GPIO_NUM_15;
const hal_gpio_num_t lcd_dc_gpio = GPIO_NUM_27;
//const hal_gpio_num_t lcd_rst_gpio;

const hal_spi_port_config_t lcd_spi_port_config = 
{
    .host = HSPI_HOST,
	.buscfg = 
    {
		.mosi_io_num = lcd_mosi_gpio,
		.miso_io_num = -1,//lcd_miso_gpio,
		.sclk_io_num = lcd_clk_gpio,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0
	},
	.devcfg = 
    {
		.mode = 0,
		    //SPI mode 0
		.clock_speed_hz = 40000 * 1000, //40MHz
		.spics_io_num = -1,
		 //CS pin
		.queue_size = 7, 
		                          //We want to be able to queue 7 transactions at a time
		//.pre_cb,
		   //Specify pre-transfer callback to handle D/C line
		//.post_cb,
		   //Specify post-transfer callback to handle CS line
    }
};


const lcd_display_config_t lcd_gpio_data = 
{
    .spi_port_config = &lcd_spi_port_config,
    .miso_gpio = NULL,//&lcd_miso_gpio,
    .mosi_gpio = &lcd_mosi_gpio,
    .clk_gpio = &lcd_clk_gpio,
    .cs_gpio = NULL,//&lcd_cs_gpio,
    .dc_gpio = &lcd_dc_gpio,
    .rst_gpio = NULL,//&lcd_rst_gpio,
};

const lcd_display_config_t* lcd_gpio_channel = &lcd_gpio_data;





void gpio_ports_init()
{
    hal_gpio_pad_select_gpio(GPIO_NUM_15);
		hal_gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
		hal_gpio_set_level(GPIO_NUM_15, 0);

    assert(hal_pwm_timer_config(lcd_bl_pwm_timer) == APP_OK);
    assert(hal_pwm_channel_config(lcd_bl_pwm_channel) == APP_OK);
}

