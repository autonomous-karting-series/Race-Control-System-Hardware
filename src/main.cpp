#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

// #define ONBOARD_LED GPIO_NUM_15
#define ONBOARD_ANTENNA GPIO_NUM_3
#define EXTERNAL_ANTENNA GPIO_NUM_14

#define GPIO_HIGH 1
#define GPIO_LOW 0

// Logging and Config
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"

#include "mesh.hpp"
#include "indicator.hpp"

#define LOOP_DLY 1000
uint8_t s_led_state = 0;

// static TaskHandle_t rx_led_handle = NULL;

static void configure_external_antenna() {
    gpio_set_direction(ONBOARD_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(ONBOARD_ANTENNA, GPIO_LOW);

    gpio_set_direction(EXTERNAL_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(EXTERNAL_ANTENNA, GPIO_HIGH);
}

// void update_led(void *arg)
// {
//     ESP_LOGI("LED", "Starting LED Task");
//     is_running = true;

//     // while (!rx_led)
//     // {
//     //     ESP_LOGI("LED", "Waiting for Semaphore");
//     //     vTaskDelay(1000 / portTICK_PERIOD_MS);
//     // }
    
//     // ESP_LOGI("LED", "Semaphore created, starting loop");
//     // ESP_LOGI("LED", "Semaphore count %d", );

//     while(is_running)
//     {
//         // if (xSemaphoreTake(rx_led, portMAX_DELAY))
//         ESP_LOGI("LED", "Waiting for notify");
//         ESP_LOGI("LED", "RX Handle is Null: %d", rx_led_handle == NULL);

//         ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

//         ESP_LOGI("LED", "blinking!");
//         // gpio_set_level(ONBOARD_LED, 1);
//         // vTaskDelay(100 / portTICK_PERIOD_MS);
        
//         // s_led_state = 0;
//         // gpio_set_level(ONBOARD_LED, 0);

//         // vTaskDelay(100 / portTICK_PERIOD_MS);
//     }

//     rx_led_handle = NULL;
//     vTaskDelete(NULL);
// }

// static void configure_led()
// {
//     gpio_reset_pin(ONBOARD_LED);
//     gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);
// }

extern "C" void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mesh", ESP_LOG_ERROR);
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set("MESH_RX", ESP_LOG_DEBUG);

    /* Initialize TCP/IP stack and system event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    configure_external_antenna();
    setup_indicator_leds();
    start_rx_led();

    initialize_wifi();
    start_mesh();

    while (1) {
        vTaskDelay(LOOP_DLY / portTICK_PERIOD_MS);
    }
}



/**
 * =========================
 *
 * Internal Mesh Messages:
 *  - MESH_PROTO_MQTT
 *  - [x] Root -> All
 *    -> MESH_TOS_P2P
 *  - [x] Leaf -> Root
 *    -> MESH_TOS_DEF
 *
 * =========================
 *
 * Leaf Nodes:
 *  - [ ] Connect to PC Connector
 *  - [ ] Forward MQTT packets to PC Connector
 *    -> [ ] ignore messages from self
 *  - [x] Create MQTT packets and send to Root
 *  - [ ] Collect GPS points
 *  - [ ] Send hardware data to MQTT
 *    -> [ ] Health Status
 *    -> [ ] Battery Percent
 *    -> [ ] GPS Coords (w/ correction?)
 *    -> [ ] Mesh Parent
 *    -> [ ] RSSI to Mesh Parent
 *  - [ ] Trigger new parent past RSSI threshold (quantify benefit and auto-healing)
 *
 * =========================
 *
 * Root Node:
 *  - [x] Create Mesh
 *  - [x] Listen for packets from internal mesh
 *    -> [x] Forward to MQTT broker
 *  - [x] Listen for MQTT messages from broker
 *    -> [x] Forward to internal mesh
 *  - [ ] Update Device IDs per team?
 *    -> [ ] Use to change mqtt Topic ID (/kart/{team}/...)
 *
 * =========================
 *
 * Non-Root Nodes (root-eligible):
 *  - [ ] Retransmit to mesh children
 */