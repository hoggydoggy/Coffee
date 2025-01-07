/*
 * Minimal Starter Code for Zigbee Coffee Machine - Step 4
 *
 * Adds a basic Zigbee On/Off cluster for controlling the relay.
 * This is a simplified example assuming you have esp-zigbee-sdk set up.
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"

/* ============== */
/* Zigbee Headers */
/* ============== */
#include "esp_zb_api.h"
#include "esp_zb_device_esp32.h"
#include "esp_zb_common.h"
#include "esp_zb_clusters.h"

/* -------------------- */
/*  PIN/Channel Defines */
/* -------------------- */
#define GPIO_POWER_RELAY   2
#define ADC1_LED_CHANNEL   ADC_CHANNEL_0

/* Zigbee Endpoint & Clusters */
#define COFFEE_ENDPOINT    10
#define HA_PROFILE_ID      0x0104       // Home Automation Profile
#define ON_OFF_CLUSTER_ID  ZB_ZCL_CLUSTER_ID_ON_OFF

/* Global relay state & function prototypes */
static bool power_relay_on = false;

static void init_coffee_machine_hardware(void);
static void init_coffee_machine_adc(void);
static void init_zigbee_stack(void);
static void start_zigbee_network(void);
static void toggle_power_relay(void);
static bool is_coffee_led_on(void);

/* ======================= */
/*  Zigbee On/Off Cluster  */
/* ======================= */

/*
 * A standard On/Off attribute list. 
 * 'ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST' is a macro that creates
 * the necessary data structures for storing the on/off attribute.
 */
ZB_ZCL_DECLARE_ON_OFF_ATTRIB_LIST(on_off_attr_list, &power_relay_on);

/*
 * Home Automation Simple Descriptor definition:
 *  - Endpoint number
 *  - Profile ID (HA_PROFILE_ID)
 *  - Application device version
 *  - Input/Output cluster arrays, etc.
 */
ZB_HA_DECLARE_SIMPLE_DESC(ha_simple_desc,
                          COFFEE_ENDPOINT,
                          HA_PROFILE_ID,     // Home Automation
                          ZB_ZCL_VERSION,    // Zigbee Cluster Library version
                          1,                 // # of Input Clusters
                          1,                 // # of Output Clusters
                          ON_OFF_CLUSTER_ID, // Input cluster #1
                          ON_OFF_CLUSTER_ID  // Output cluster #1
);

/*
 * Device context: ties together the attribute list, cluster definitions, etc.
 * For more advanced usage, you might define multiple clusters, or multiple endpoints.
 */
ZB_HA_DECLARE_DEVICE_CTX_1_EP(coffee_device_ctx,
                              ha_simple_desc,
                              COFFEE_ENDPOINT,
                              on_off_attr_list,
                              NULL /* no extra cluster list here */);

/* ============================================ */
/*  Zigbee Command Handler for On/Off Commands  */
/* ============================================ */
/*
 * This callback is invoked when a Zigbee On/Off command is received
 * (On, Off, or Toggle). We'll just link it to our 'toggle_power_relay()'.
 */
static zb_void_t on_off_cluster_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    switch (cmd_info.cmd_id) {
    case ZB_ZCL_CMD_ON_OFF_ON_ID:
        power_relay_on = true;
        gpio_set_level(GPIO_POWER_RELAY, 1);
        printf("[on_off_cluster_handler] Received ON command, relay ON.\n");
        break;

    case ZB_ZCL_CMD_ON_OFF_OFF_ID:
        power_relay_on = false;
        gpio_set_level(GPIO_POWER_RELAY, 0);
        printf("[on_off_cluster_handler] Received OFF command, relay OFF.\n");
        break;

    case ZB_ZCL_CMD_ON_OFF_TOGGLE_ID:
        toggle_power_relay();
        printf("[on_off_cluster_handler] Received TOGGLE command.\n");
        break;

    default:
        printf("[on_off_cluster_handler] Unknown On/Off command ID: %d\n", cmd_info.cmd_id);
        break;
    }

    /* Free the Zigbee buffer after processing */
    zb_buf_free(param);
}

/*
 * This function registers the On/Off cluster handler
 * so the Zigbee stack knows to call it when On/Off commands arrive.
 */
static void register_on_off_cluster_handler(void)
{
    zb_ret_t zb_err = zb_zcl_add_cluster_handlers(ON_OFF_CLUSTER_ID,
                                                  NULL, /* No cluster init function */
                                                  NULL, /* No attribute handler */
                                                  on_off_cluster_handler);
    if (zb_err == RET_OK) {
        printf("[register_on_off_cluster_handler] On/Off cluster handler registered.\n");
    } else {
        printf("[register_on_off_cluster_handler] Error registering On/Off cluster handler: %d\n", zb_err);
    }
}

/* ================================== */
/*  Zigbee Stack & Networking Logic   */
/* ================================== */

/*
 * We'll create a callback to handle general Zigbee stack events:
 * - joined network
 * - attribute writes
 * - etc.
 */
static void zb_zigbee_event_handler(zb_bufid_t bufid)
{
    zb_zdo_app_signal_hdr_t *signal_hdr = NULL;
    zb_zdo_app_signal_type_t sig_type = zb_get_app_signal(bufid, &signal_hdr);
    zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);

    switch (sig_type) {
    case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        if (status == RET_OK) {
            printf("[zb_zigbee_event_handler] Device joined network successfully.\n");
        } else {
            printf("[zb_zigbee_event_handler] Device failed to join network.\n");
        }
        break;
    default:
        break;
    }

    zb_free_buf(bufid);
}

/*
 * Minimal Zigbee initialization. We:
 *  1. Call esp_zb_init() with our device context
 *  2. Register the On/Off cluster handler
 */
static void init_zigbee_stack(void)
{
    printf("[init_zigbee_stack] Zigbee stack initialization started...\n");

    /* Initialize Zigbee device with the declared device context (coffee_device_ctx) */
    esp_zb_init(&coffee_device_ctx);

    /* Register On/Off cluster commands */
    register_on_off_cluster_handler();

    printf("[init_zigbee_stack] Zigbee stack initialization complete.\n");
}

/*
 * Start or join a Zigbee network as an End Device. 
 * This triggers network commissioning if not already joined.
 */
static void start_zigbee_network(void)
{
    printf("[start_zigbee_network] Attempting to join or form Zigbee network...\n");

    /* By default, let's set as an End Device. If you want a router, change accordingly. */
    esp_zb_set_network_role(ZB_NWK_DEVICE_TYPE_ED);

    /* Register a callback to handle Zigbee stack events */
    esp_zb_device_register_join_callback(zb_zigbee_event_handler);

    /* Start the Zigbee stack - param true means start automatically */
    esp_zb_start(true);

    printf("[start_zigbee_network] Zigbee network start invoked.\n");
}

/* ========================== */
/*        app_main           */
/* ========================== */
void app_main(void)
{
    printf("\n=== Zigbee Coffee Machine - Step 4: On/Off Cluster ===\n\n");

    /* Standard hello-world style chip/flash info */
    esp_chip_info_t chip_info;
    uint32_t flash_size = 0;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s)\n", CONFIG_IDF_TARGET, chip_info.cores);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed.\n");
    } else {
        printf("Flash size: %" PRIu32 "MB\n", flash_size / (1024 * 1024));
    }
    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    /* Initialize hardware for coffee machine control */
    init_coffee_machine_hardware();

    /* Initialize ADC to read a coffee machine LED (from Step 3) */
    init_coffee_machine_adc();

    /* Initialize Zigbee stack (our cluster definitions) */
    init_zigbee_stack();

    /* Start or join a Zigbee network */
    start_zigbee_network();

    /* --------------------------------- */
    /* Demo: Toggle relay & read the LED */
    /* --------------------------------- */
    for (int i = 0; i < 5; i++) {
        printf("\n[app_main] Toggling the power relay in 1 second...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        toggle_power_relay();

        /* Check LED for demonstration (from Step 3) */
        bool led_state = is_coffee_led_on();
        printf("[app_main] Coffee LED is currently: %s\n",
               led_state ? "ON" : "OFF");
    }

    /* ---------------------------------------- */
    /* Existing countdown logic, then restart.  */
    /* ---------------------------------------- */
    for (int i = 10; i >= 0; i--) {
        printf("[app_main] Running... Restart in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("[app_main] Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

/* ====================== */
/*  Hardware Functions    */
/* ====================== */
static void init_coffee_machine_hardware(void)
{
    printf("[init_coffee_machine_hardware] Configuring GPIO pins...\n");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_POWER_RELAY),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    /* Initialize relay to OFF */
    gpio_set_level(GPIO_POWER_RELAY, 0);
    power_relay_on = false;

    printf("[init_coffee_machine_hardware] Finished configuring GPIO pin %d for power relay.\n", GPIO_POWER_RELAY);
}

static void init_coffee_machine_adc(void)
{
    printf("[init_coffee_machine_adc] Initializing ADC...\n");
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_LED_CHANNEL, ADC_ATTEN_DB_11);
    printf("[init_coffee_machine_adc] ADC channel %d initialized.\n", ADC1_LED_CHANNEL);
}

/* 
 * Reads the LED raw value. 
 * We'll treat above 2000 (out of 4095) as "LED ON" - tune as needed.
 */
static bool is_coffee_led_on(void)
{
    int raw = adc1_get_raw(ADC1_LED_CHANNEL);
    bool led_on = (raw > 2000);
    printf("[is_coffee_led_on] Raw ADC: %d => LED %s\n", raw, led_on ? "ON" : "OFF");
    return led_on;
}

/*
 * Simple function that toggles the power relay ON/OFF
 */
static void toggle_power_relay(void)
{
    power_relay_on = !power_relay_on; 
    gpio_set_level(GPIO_POWER_RELAY, power_relay_on);
    printf("[toggle_power_relay] Power Relay is now %s\n", 
           power_relay_on ? "ON" : "OFF");
}
