idf_component_register(
    SRCS "coffee_zb_main.c"
    INCLUDE_DIRS "."
    REQUIRES 
      driver
      nvs_flash
      esp-zigbee-lib    # <-- Must match the actual COMPONENT_NAME if it's "esp-zigbee-lib"
)