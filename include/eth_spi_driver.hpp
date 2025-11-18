#ifndef ETH_SPI_DRIVER
#define ETH_SPI_DRIVER

#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

#include "esp_eth_driver.h"
#include "esp_eth_phy_w5500.h"
#include "esp_eth_mac_w5500.h"
// #include "driver/spi_master.h"
#include "driver/gpio.h"

#define ETH_HOST SPI1_HOST
#define SPI_MISO GPIO_NUM_20
#define SPI_MOSI GPIO_NUM_18
#define SPI_CLK GPIO_NUM_19
#define SPI_CS GPIO_NUM_21
#define ETH_MAX_BYTES 80
#define ETH_SPI_CLOCK_MHZ 16

typedef struct {
    uint8_t spi_cs_gpio;
    int8_t int_gpio;
    uint32_t polling_ms;
    int8_t phy_reset_gpio;
    uint8_t phy_addr;
    uint8_t *mac_addr;
}spi_eth_module_config_t;

static const char ETH_TAG[] = "ETH_SPI";
static bool gpio_isr_svc_init_by_eth = false; // indicates that we initialized the GPIO ISR service

/**
 * @brief SPI bus initialization (to be used by Ethernet SPI modules)
 *
 * @return
 *          - ESP_OK on success
 */
esp_err_t spi_bus_init();

/**
 * @brief Ethernet SPI modules initialization
 *
 * @param[in] spi_eth_module_config specific SPI Ethernet module configuration
 * @param[out] mac_out optionally returns Ethernet MAC object
 * @param[out] phy_out optionally returns Ethernet PHY object
 * @return
 *          - esp_eth_handle_t if init succeeded
 *          - NULL if init failed
 */
esp_eth_handle_t eth_init_spi(spi_eth_module_config_t *spi_eth_module_config, esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out);

#endif