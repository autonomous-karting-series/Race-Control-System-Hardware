#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

#include "mesh.hpp"
#include "indicator.hpp"
#include "eth_spi_driver.hpp"

#define ONBOARD_ANTENNA GPIO_NUM_3
#define EXTERNAL_ANTENNA GPIO_NUM_14

#define GPIO_HIGH 1
#define GPIO_LOW 0
#define LOOP_DLY 1000

void configure_external_antenna() {
    // esp_phy_set_ant_gpio();

    gpio_set_direction(ONBOARD_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(ONBOARD_ANTENNA, GPIO_LOW);

    gpio_set_direction(EXTERNAL_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(EXTERNAL_ANTENNA, GPIO_HIGH);
}

extern "C" void app_main(void)
{
    // Reduce mesh and wifi log bloat
    esp_log_level_set("mesh", ESP_LOG_WARN);
    esp_log_level_set("wifi", ESP_LOG_WARN);

    // Active Debug
    esp_log_level_set("MESH_RX", ESP_LOG_DEBUG);
    esp_log_level_set("w5500*", ESP_LOG_DEBUG);
    esp_log_level_set("esp_eth*", ESP_LOG_DEBUG);

    /* Initialize TCP/IP stack and system event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Hardware Initialization
    configure_external_antenna();
    setup_indicator_leds();
    // setup_eth_gpio();
    
    // RX LED Indicator needs to be initialized before mesh
    initialize_wifi();
    init_eth();
    start_mesh();

    while (1) {
        // Not doing anything in main task
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
 *    -> [ ] Create timer for health watchdog that resets every message from connector
 *  - [ ] Forward MQTT packets to PC Connector
 *    -> [x] ignore messages from self
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