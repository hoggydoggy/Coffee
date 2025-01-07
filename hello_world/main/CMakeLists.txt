idf_component_register(
    SRCS "hello_world_main.c"   # <-- Source file(s) in this directory
    PRIV_REQUIRES spi_flash
    INCLUDE_DIRS "."            # <-- Where to look for header files
    REQUIRES driver             # <-- This ensures the 'driver' component (GPIO, etc.) is included
)

