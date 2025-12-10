#ifndef ETH_SPI_DRIVER
#define ETH_SPI_DRIVER

#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "sdkconfig.h"

#include "esp_netif.h"
#include "esp_eth.h"
#include "ethernet_init.h"

#include "driver/gpio.h"

#define START_ETH_GPIO GPIO_NUM_2

static const char ETH_TAG[] = "ETH_SPI";

static bool eth_task_started = false;
static bool eth_driver_started = false;
static uint8_t eth_port_cnt = 0;
static esp_eth_handle_t* eth_handle;
static SemaphoreHandle_t eth_start_handle;

void setup_eth_gpio();
void init_eth();

#endif