#include "app_hal_spi.h"
#include "system_include.h"


void hal_spi_port_init(const hal_spi_port_config_t* spi_port_config, hal_spi_port_t spi_port)
{
	if (spi_port_config!=NULL)
	{
		esp_err_t ret = spi_bus_initialize(spi_port_config->host, &spi_port_config->buscfg, 1);
		ESP_ERROR_CHECK(ret);
	}
	
	if (spi_port!=NULL)
	{
		esp_err_t ret = spi_bus_add_device(spi_port_config->host, &spi_port_config->devcfg, spi_port);
		ESP_ERROR_CHECK(ret);
	}
}
