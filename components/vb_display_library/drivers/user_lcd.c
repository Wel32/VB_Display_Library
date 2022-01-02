#include "user_lcd.h"



spi_device_handle_t spi;
uint8_t spi_part_of_transaction_started = 0;
const lcd_display_config_t* display_interface_config = NULL;


inline void lcd_start_transfer(int mode)
{
	if (mode <= 1)
	{
		hal_gpio_set_level(*display_interface_config->dc_gpio, mode);

		if (!mode) 
		{
			if (display_interface_config->cs_gpio!=NULL) hal_gpio_set_level(*display_interface_config->cs_gpio, 0);
		}
	}
}

inline void internal_lcd_end_transfer()
{
	if (display_interface_config->cs_gpio!=NULL) hal_gpio_set_level(*display_interface_config->cs_gpio, 1);
	else hal_gpio_set_level(*display_interface_config->dc_gpio, 0);
}

void lcd_end_transfer()
{
#if LCD_SPI_ENABLE_DMA
	if (spi_part_of_transaction_started)
	{
		spi_part_of_transaction_started = 0;
		spi_device_polling_end(spi, portMAX_DELAY);
	}
#endif

	internal_lcd_end_transfer();
}


inline int lcd_isReady()
{
	return hal_gpio_get_level(*display_interface_config->cs_gpio);
}


void lcd_send_cmd(uint8_t cmd)
{
	if (display_interface_config->cs_gpio!=NULL)
	{
		if (!lcd_isReady()) 
		{
			if (hal_gpio_get_level(*display_interface_config->dc_gpio)) while (!lcd_isReady()) ;//sending data
			else  lcd_end_transfer(); //sending cmd
		}
	}
	else while (hal_gpio_get_level(*display_interface_config->dc_gpio));

#if LCD_SPI_ENABLE_DMA
	spi_part_of_transaction_started = 0;
#endif

	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));        //Zero out the transaction
	t.length = 8;                      //Command is 8 bits
	t.tx_buffer = &cmd;                //The data is the cmd itself
	t.user = (void*)0;                 //D/C needs to be set to 0

	if (display_interface_config->cs_gpio!=NULL) hal_gpio_set_level(*display_interface_config->cs_gpio, 0);

	ret = spi_device_polling_transmit(spi, &t);   //Transmit!
	assert(ret == ESP_OK);             //Should have had no issues.
}

void lcd_send_data(uint8_t *data, int len)
{
	esp_err_t ret;
	spi_transaction_t t;
	if (len == 0) return;             //no need to send anything
	memset(&t, 0, sizeof(t));        //Zero out the transaction
	t.length = len * 8;                  //Len is in bytes, transaction length is in bits.
	t.tx_buffer = data;                //Data
	t.user = (void*)1;                 //D/C needs to be set to 1
	ret = spi_device_polling_transmit(spi, &t);   //Transmit!
	assert(ret == ESP_OK);             //Should have had no issues.
}

void lcd_send_part_of_data(uint8_t *data, int len)
{
	esp_err_t ret;

#if LCD_SPI_ENABLE_DMA

	static spi_transaction_t trans_data;

	if (spi_part_of_transaction_started) spi_device_polling_end(spi, portMAX_DELAY);
	else spi_part_of_transaction_started = 1;

	if (len == 0) return;             //no need to send anything
	memset(&trans_data, 0, sizeof(trans_data));        //Zero out the transaction
	trans_data.length = len * 8;                  //Len is in bytes, transaction length is in bits.
	trans_data.tx_buffer = data;                //Data
	trans_data.user = (void*)2;                 //D/C needs to be set to 1 and not set CS to 1 after transaction
    
	ret = spi_device_polling_start(spi, &trans_data, portMAX_DELAY);

#else

	spi_transaction_t t;

	if (len == 0) return;             //no need to send anything
	memset(&t, 0, sizeof(t));        //Zero out the transaction
	t.length = len * 8;                  //Len is in bytes, transaction length is in bits.
	t.tx_buffer = data;                //Data
	t.user = (void*)2;                 //D/C needs to be set to 1 and not set CS to 1 after transaction

	ret = spi_device_polling_transmit(spi, &t);   //Transmit!

#endif

	    assert(ret == ESP_OK);             //Should have had no issues.
}


void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
	lcd_start_transfer((int)t->user);
}

void lcd_spi_post_transfer_callback(spi_transaction_t *t)
{
	int dc = (int)t->user;
	if (dc == 1) internal_lcd_end_transfer();
}


//Initialize the display interface
void lcd_interface_init(const lcd_display_config_t* init_display_interface_config)
{
	assert(init_display_interface_config!=NULL);
	display_interface_config = init_display_interface_config;

	//Initialize non-SPI GPIOs
	if (display_interface_config->dc_gpio!=NULL)
	{
		hal_gpio_pad_select_gpio(*display_interface_config->dc_gpio);
		hal_gpio_set_direction(*display_interface_config->dc_gpio, GPIO_MODE_OUTPUT);
		hal_gpio_set_level(*display_interface_config->dc_gpio, 0);
	}
	if (display_interface_config->rst_gpio!=NULL)
	{
		hal_gpio_pad_select_gpio(*display_interface_config->rst_gpio);
		hal_gpio_set_direction(*display_interface_config->rst_gpio, GPIO_MODE_OUTPUT);
		hal_gpio_set_level(*display_interface_config->rst_gpio, 0);
	}

	if (display_interface_config->cs_gpio!=NULL)
	{
		hal_gpio_pad_select_gpio(*display_interface_config->cs_gpio);
		hal_gpio_set_direction(*display_interface_config->cs_gpio, GPIO_MODE_OUTPUT);
		hal_gpio_set_level(*display_interface_config->cs_gpio, 0);
	}

	app_err_t ret;

	spi_device_interface_config_t devcfg = display_interface_config->spi_port_config->devcfg;
	devcfg.pre_cb = lcd_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line
	devcfg.post_cb = lcd_spi_post_transfer_callback; //Specify post-transfer callback to handle CS line


	//Initialize the SPI bus
	ret = spi_bus_initialize(display_interface_config->spi_port_config->host, &(display_interface_config->spi_port_config->buscfg), 1);
	APP_ERROR_CHECK(ret);

	//Attach the LCD to the SPI bus
	ret = spi_bus_add_device(display_interface_config->spi_port_config->host, &devcfg, &spi);
	APP_ERROR_CHECK(ret);


	if (display_interface_config->rst_gpio!=NULL)
	{
		vTaskDelay(250 / portTICK_RATE_MS);
		hal_gpio_set_level(*display_interface_config->rst_gpio, 1);
	}

	if (display_interface_config->cs_gpio!=NULL)
	{
		hal_gpio_set_level(*display_interface_config->dc_gpio, 1);
		hal_gpio_set_level(*display_interface_config->dc_gpio, 0);
	}
}


uint8_t lcd_set_orient(uint8_t orient)
{
	if (!display_is_initialized) return 0;
	
	lcd_internal_set_orient(orient);
	
	return 1;
}


