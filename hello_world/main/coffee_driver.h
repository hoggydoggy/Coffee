#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize hardware for the coffee machine
 */
void coffee_driver_init(void);

/**
 * @brief Set coffee machine power (relay) ON or OFF
 */
void coffee_driver_set_power(bool on);

/**
 * @brief This is a callback from Zigbee when an attribute changes (like On/Off).
 *        We connect it to the On/Off cluster.
 */
esp_err_t esp_zb_attribute_callback(esp_zb_core_action_callback_id_t callback_id, const void *message);

#ifdef __cplusplus
}
#endif
