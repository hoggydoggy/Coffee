cmake_minimum_required(VERSION 3.5)

# Include Espressif's project.cmake from your ESP-IDF installation
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Name your project
project(HELLO_WORLD)

# (Optional) If you want to set default sdkconfig values, you could do:
# set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")

# If you cloned the esp-zigbee-sdk into a 'components' folder, do:
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components/esp_zigbee_sdk")
#
# If you're using idf.py add-dependency, you might not need the above line.

# Nothing else needed here. The actual build instructions are in main/CMakeLists.txt