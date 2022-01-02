

idf_component_register(SRCS "user_menu_mashine.cpp"
                            "user_menu_stack.cpp"
                                INCLUDE_DIRS "."
			     REQUIRES "app_hal"
                    )

#set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY LINK_INTERFACE_MULTIPLICITY 5)