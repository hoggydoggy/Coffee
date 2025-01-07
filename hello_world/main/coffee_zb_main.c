/*
 * coffee_zb_main.c
 *
 * A minimal Zigbee End-Device example that:
 *  1. Initializes the Zigbee platform (radio + host).
 *  2. Creates an On/Off Light endpoint as an example (renamed as "coffee" here).
 *  3. Joins the Zigbee network on the primary channels.
 * 
 * IMPORTANT:
 *  - In menuconfig, set ZB_ED_ROLE (End Device).
 *  - Adjust the component name in CMakeLists.txt to ensure "esp_zb_light.h" 
 *    and "ha/esp_zigbee_ha_standard.h" are found.
 *  - If you have a custom driver for coffee machine relays, call it from here.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

/* Zigbee includes */
#include "ha/esp_zigbee_ha_standard.h" /* If your library organizes HA standard includes here */
#include "esp_zb_light.h"              /* Typically contains On/Off Light macros, e.g., ZED configs */

static const char *TAG = "coffee_zb_main";

/* Forward declaration of the Zigbee main task */
static void coffee_zb_task(void *pvParameters);

/******************************************************************************
 * Function: app_main
 *    The standard ESP-IDF entry point. Initializes NVS, configures Zigbee,
 *    and creates a Zigbee task.
 ******************************************************************************/
void app_main(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "==== Starting Coffee Zigbee End-Device ====");

    /* Initialize NVS (required by Zigbee to store network parameters) */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Configure Zigbee platform: set radio/host config 
     * These macros require ZB_ED_ROLE to be set in menuconfig.
     */
    esp_zb_platform_config_t zb_config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&zb_config));

    /* Create a dedicated Zigbee task so the stack has its own main loop. */
    xTaskCreate(coffee_zb_task, "coffee_zb_task", 4096, NULL, 5, NULL);
}

/******************************************************************************
 * Function: coffee_zb_task
 *    The primary Zigbee logic: 
 *      1. Initialize as a ZED (End Device) 
 *      2. Create a standard On/Off Light endpoint
 *      3. Register the endpoint + set channels
 *      4. Start the Zigbee stack event loop
 ******************************************************************************/
static void coffee_zb_task(void *pvParameters)
{
    ESP_LOGI(TAG, "coffee_zb_task started");

    /* 1. Initialize Zigbee as an End Device 
     *    This macro is valid only if ZB_ED_ROLE is enabled 
     */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    /* 2. Create a default On/Off Light config. 
     *    We'll adapt it for coffee control, but the cluster is still 'On/Off'.
     */
    esp_zb_on_off_light_cfg_t on_off_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();

    /* 3. Create endpoint and register it with the Zigbee stack
     *    HA_ESP_LIGHT_ENDPOINT is a macro (e.g., 1) from esp_zb_light.h
     */
    esp_zb_ep_list_t *coffee_ep = esp_zb_on_off_light_ep_create(HA_ESP_LIGHT_ENDPOINT, &on_off_cfg);
    esp_zb_device_register(coffee_ep);

    /* 4. Set the primary channels we want to try for joining 
     *    e.g., 1, 11, 15, 20, etc. The macro covers a default set.
     */
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);

    /* 5. Start the Zigbee stack. The 'false' param means no blocking. 
     *    Then we enter the main event loop, which never returns.
     */
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();

    /* If we ever exit the main loop, tidy up. (Unlikely in normal operation.) */
    vTaskDelete(NULL);
}
