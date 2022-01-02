

idf_component_register(SRCS 	"app_hal_api.c"
                            	"app_hal_gpio/app_hal_gpio.c"
				"app_hal_pwm/app_hal_pwm.c"
                    		"app_hal_spi/app_hal_spi.c"

                    		INCLUDE_DIRS "."
			     	REQUIRES "spi_flash"
                    )

#set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY LINK_INTERFACE_MULTIPLICITY 5)