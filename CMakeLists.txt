cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(GPIO)
add_compile_options(-fexceptions)

set(COMPONENT_SRCS "main/main.cpp" REQUIRED esp_driver_spi)