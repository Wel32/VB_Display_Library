#include "user_lcd.h"



spi_device_handle_t spi;
uint8_t spi_part_of_transaction_started = 0;



inline void lcd_start_transfer(int mode)
{
	if (mode <= 1)
	{
		gpio_set_level(PIN_NUM_DC, mode);
#ifdef PIN_NUM_CS
		if (!mode) gpio_set_level(PIN_NUM_CS, 0);
#endif
	}
}

inline void internal_lcd_end_transfer()
{
#ifdef PIN_NUM_CS
	gpio_set_level(PIN_NUM_CS, 1);
#else
	gpio_set_level(PIN_NUM_DC, 0);
#endif
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

#ifdef PIN_NUM_CS
inline int lcd_isReady()
{
	return gpio_get_level(PIN_NUM_CS);
}
#endif

void lcd_send_cmd(uint8_t cmd)
{
#ifdef PIN_NUM_CS
	if (!lcd_isReady()) 
	{
		if (gpio_get_level(PIN_NUM_DC)) while (!lcd_isReady()) ;//sending data
		else  lcd_end_transfer(); //sending cmd
	}
#else
	while (gpio_get_level(PIN_NUM_DC));
#endif

#if LCD_SPI_ENABLE_DMA
	spi_part_of_transaction_started = 0;
#endif

	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));        //Zero out the transaction
	t.length = 8;                      //Command is 8 bits
	t.tx_buffer = &cmd;                //The data is the cmd itself
	t.user = (void*)0;                 //D/C needs to be set to 0

#ifdef PIN_NUM_CS
	gpio_set_level(PIN_NUM_CS, 0);
#endif

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
void lcd_interface_init()
{
	//Initialize non-SPI GPIOs
	gpio_pad_select_gpio(PIN_NUM_DC);
#ifdef PIN_NUM_RST
	gpio_pad_select_gpio(PIN_NUM_RST);
#endif
#ifdef PIN_NUM_CS
	gpio_pad_select_gpio(PIN_NUM_CS);
#endif
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
#ifdef PIN_NUM_RST
	gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
#endif
#ifdef PIN_NUM_CS
	gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
#endif

#ifdef PIN_NUM_RST
	gpio_set_level(PIN_NUM_RST, 0);
#endif
#ifdef PIN_NUM_CS
	gpio_set_level(PIN_NUM_CS, 0);
#endif
    
    

	esp_err_t ret;
    
	spi_bus_config_t buscfg =  {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0
	};

	spi_device_interface_config_t devcfg =  {
		.mode = 0,
		    //SPI mode 0
		.clock_speed_hz = LCD_SPI_RATE_kHz * 1000,
		.spics_io_num = -1,
		 //PIN_NUM_CS,               //CS pin
		.queue_size = 7, 
		                          //We want to be able to queue 7 transactions at a time
		.pre_cb = lcd_spi_pre_transfer_callback,
		   //Specify pre-transfer callback to handle D/C line
		.post_cb = lcd_spi_post_transfer_callback,
		   //Specify post-transfer callback to handle CS line
	};


	//Initialize the SPI bus
	ret = spi_bus_initialize(LCD_SPI_CHANNEL, &buscfg, 1);
	ESP_ERROR_CHECK(ret);
	//Attach the LCD to the SPI bus
	ret = spi_bus_add_device(LCD_SPI_CHANNEL, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);
	
#ifdef PIN_NUM_RST
	vTaskDelay(250 / portTICK_RATE_MS);
	gpio_set_level(PIN_NUM_RST, 1);
#endif
	
#ifndef PIN_NUM_CS
	gpio_set_level(PIN_NUM_DC, 1);
	gpio_set_level(PIN_NUM_DC, 0);
#endif
}







uint8_t lcd_set_orient(uint8_t orient)
{
	if (!display_is_initialized) return 0;
	
	lcd_internal_set_orient(orient);
	
	return 1;
}


