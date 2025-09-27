#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO
#include "driver/gpio.h"

#define ONBOARD_LED GPIO_NUM_15
#define ONBOARD_ANTENNA GPIO_NUM_3
#define EXTERNAL_ANTENNA GPIO_NUM_14

#define GPIO_HIGH 1
#define GPIO_LOW 0

// Logging and Config
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"

// #include "mqtt_client.h"

#include "mesh.hpp"

#define LOOP_DLY 1000
static uint8_t s_led_state = 0;

static void configure_external_antenna() {
    gpio_set_direction(ONBOARD_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(ONBOARD_ANTENNA, GPIO_LOW);

    gpio_set_direction(EXTERNAL_ANTENNA, GPIO_MODE_OUTPUT);
    gpio_set_level(EXTERNAL_ANTENNA, GPIO_HIGH);
}

static void blink_led()
{
    gpio_set_level(ONBOARD_LED, s_led_state);
}

static void configure_led()
{
    gpio_reset_pin(ONBOARD_LED);
    gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);
}


extern "C" void app_main(void)
{
    configure_external_antenna();

    configure_led();




    mesh_controller::start_mesh();

    while (1) {
        // ESP_LOGI("Main Loop", "Running in main loop!");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(LOOP_DLY / portTICK_PERIOD_MS);
    }
}