/*
 * coffee_zb_main.c
 *
 * Minimal Zigbee On/Off End-Device for a "Coffee Machine"
 * using the new Espressif Zigbee approach (HA_on_off_light style).
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

/* Zigbee headers from the new approach */
#include "ha/esp_zigbee_ha_standard.h"

/* Our coffee relay driver header */
#include "coffee_driver.h"

static const char *TAG = "coffee_zb_main";

/* Forward declaration of the Zigbee task */
static void coffee_zb_task(void *pvParameters);

/*********************************************************************
 * @brief app_main
 *        The top-level entry point of the program.
 *        Initializes Zigbee platform, spawns a Zigbee task.
 *********************************************************************/
void app_main(void)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "==== Starting Coffee Zigbee End-Device ====");

    /* Initialize NVS - required by Zigbee stack for storing network params */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Configure Zigbee platform */
    esp_zb_platform_config_t zb_config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&zb_config));

    /* Create a dedicated Zigbee task, as recommended by the official example */
    xTaskCreate(coffee_zb_task, "coffee_zb_task", 4096, NULL, 5, NULL);
}

/*********************************************************************
 * @brief coffee_zb_task
 *        The main Zigbee logic: device init, endpoint creation, start.
 *********************************************************************/
static void coffee_zb_task(void *pvParameters)
{
    ESP_LOGI(TAG, "coffee_zb_task started");

    /* 1. Initialize the Zigbee stack as an End Device (ZED). 
     * If you want a Router, use ESP_ZB_ZR_CONFIG() instead. */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    /* 2. Create a default On/Off light config
     *    We'll adapt it to be our "coffee machine" device 
     *    but still using on/off cluster for demonstration */
    esp_zb_on_off_light_cfg_t on_off_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    /* Alternatively, you might define your own custom device if you want 
     * different clusters, but let's keep it simple. */

    /* 3. Create an endpoint that uses the on/off cluster 
     *    The official example uses HA_ESP_LIGHT_ENDPOINT (which is 1 by default). 
     *    We'll keep the same or define a new endpoint number (like 10). 
     */
    esp_zb_ep_list_t *coffee_ep = esp_zb_on_off_light_ep_create(HA_ESP_LIGHT_ENDPOINT, &on_off_cfg);

    /* 4. Register the endpoint/device with the Zigbee stack */
    esp_zb_device_register(coffee_ep);

    /* 5. Optionally, register attribute callbacks if you want to override
     *    how On/Off commands are processed. If not, the default on/off handler 
     *    will toggle an internal boolean. We’ll show how to do that in coffee_driver. 
     */
    esp_zb_core_action_handler_register(esp_zb_attribute_callback);

    /* 6. Set the Zigbee primary channel mask (e.g., 15, 20, etc.)
     *    or keep the default. Example from the official sample: 
     */
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);

    /* 7. Start the Zigbee stack (false => no blocking). 
     *    Then call the main event loop. This never returns. 
     */
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();
    /* 8. If we ever exit the main loop (unlikely), delete the task. */
    vTaskDelete(NULL);
}
