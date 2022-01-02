#pragma once
#include "app_hal_api.h"
	
#if defined(__cplusplus)
extern "C" {
#endif


typedef struct
{
	spi_host_device_t host;
	spi_bus_config_t buscfg;
	spi_device_interface_config_t devcfg;
}
hal_spi_port_config_t;

typedef spi_device_handle_t* hal_spi_port_t;


void hal_spi_port_init(const hal_spi_port_config_t* spi_port_config, hal_spi_port_t spi_port);


#if defined(__cplusplus)
}
#endif

