cmake_minimum_required(VERSION 3.5)

# Include Espressif's project.cmake from your ESP-IDF installation
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Name your project
project(HELLO_WORLD)

# Set the extra component directories
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_lib"
                         "${CMAKE_CURRENT_LIST_DIR}/components/espressif__esp-zboss-lib")

# Ensure the esp_zigbee_lib component directory exists
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_lib")
  message(FATAL_ERROR "Component directory ${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_lib does not exist.")
endif()

# Ensure the espressif__esp-zboss-lib component directory exists
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/components/espressif__esp-zboss-lib")
  message(FATAL_ERROR "Component directory ${CMAKE_CURRENT_LIST_DIR}/components/espressif__esp-zboss-lib does not exist.")
endif()

# Add the esp_zigbee_lib component explicitly
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_lib")

# Nothing else needed here. The actual build instructions are in main/CMakeLists.txt