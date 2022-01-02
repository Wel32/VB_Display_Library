

file(GLOB_RECURSE SOURCES *.c *.cpp)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES "app_hal"
                    REQUIRES "vb_display_library"
                    REQUIRES "user_menu_mashine"
                    )