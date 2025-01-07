#include "coffee_driver.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "coffee_driver";

/* Example: Use GPIO2 for coffee machine relay */
#define COFFEE_MACHINE_RELAY_PIN 2

/**
 * @brief Initialize the coffee machine hardware (GPIO)
 */
void coffee_driver_init(void)
{
    ESP_LOGI(TAG, "Initializing coffee machine hardware (GPIO)...");
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << COFFEE_MACHINE_RELAY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Start relay OFF
    gpio_set_level(COFFEE_MACHINE_RELAY_PIN, 0);
}

/**
 * @brief Turn the coffee machine relay ON or OFF
 */
void coffee_driver_set_power(bool on)
{
    gpio_set_level(COFFEE_MACHINE_RELAY_PIN, on ? 1 : 0);
    ESP_LOGI(TAG, "Coffee Machine Relay => %s", on ? "ON" : "OFF");
}

/**
 * @brief Zigbee attribute callback
 *        Called when On/Off cluster attribute changes
 */
esp_err_t esp_zb_attribute_callback(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID: {
        // "Set Attribute" callback
        const esp_zb_zcl_set_attr_value_message_t *msg = (esp_zb_zcl_set_attr_value_message_t *)message;
        if (!msg) {
            ESP_LOGE(TAG, "Attribute callback with empty message!");
            return ESP_FAIL;
        }
        if (msg->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT &&
            msg->info.cluster     == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF &&
            msg->attribute.id     == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
            msg->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
            
            // The new On/Off state is in msg->attribute.data.value
            bool new_state = *(bool*)msg->attribute.data.value;
            ESP_LOGI(TAG, "Zigbee On/Off changed => %s", new_state ? "ON" : "OFF");
            coffee_driver_set_power(new_state);
        }
        break;
    }
    default:
        // For other cluster actions, just log or do nothing
        ESP_LOGI(TAG, "Received Zigbee action callback: 0x%x", callback_id);
        break;
    }
    return ESP_OK;
}
