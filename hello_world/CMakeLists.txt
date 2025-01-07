cmake_minimum_required(VERSION 3.5)

# Include Espressif's project.cmake from your ESP-IDF installation
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Name your project
project(HELLO_WORLD)

# Set the extra component directories
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_lib")

# Add the required component
idf_component_register(SRCS "coffee_zb_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp-zigbee-lib esp-zboss-lib)