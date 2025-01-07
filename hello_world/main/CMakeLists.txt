idf_component_register(
    SRCS 
        "coffee_zb_main.c"
        "coffee_driver.c"
    INCLUDE_DIRS "."
    REQUIRES 
        driver 
        nvs_flash 
        # <-- Zigbee component name from esp-zigbee-sdk:
        
)