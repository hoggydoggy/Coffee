idf_component_register(
    SRCS "coffee_zb_main.c"
    INCLUDE_DIRS "."
    REQUIRES 
      nvs_flash
      driver
      esp-zigbee-lib
      esp_zb_light   # <-- We just created this component
)