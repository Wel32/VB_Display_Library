# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)


set(CMAKE_CXX_STANDARD 17)


set(EXTRA_COMPONENT_DIRS 
			"components/app_hal"
			"components/vb_display_library"
			"components/user_menu_mashine"
			"app"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(vb_display_library_test2)