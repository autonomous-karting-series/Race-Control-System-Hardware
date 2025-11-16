#ifndef INDICATOR
#define INDICATOR

#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

#include "driver/gpio.h"

#define ONBOARD_LED GPIO_NUM_15
#define RX_LED GPIO_NUM_16

esp_err_t setup_indicator_leds();
esp_err_t start_rx_led();
esp_err_t notify_rx_led();

static const char* RX_LED_TAG = "INDICATOR";
static const int RX_LED_DELAY = 100;

static TaskHandle_t rx_led_handle;

#endif