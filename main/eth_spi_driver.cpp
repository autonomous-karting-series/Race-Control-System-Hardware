#include "eth_spi_driver.hpp"

esp_err_t spi_bus_init()
{
    esp_err_t ret = ESP_OK;

    // // Install GPIO ISR handler to be able to service SPI Eth modules interrupts
    // ret = gpio_install_isr_service(0);
    // if (ret == ESP_OK) {
    //     gpio_isr_svc_init_by_eth = true;
    // } else if (ret == ESP_ERR_INVALID_STATE) {
    //     ESP_LOGW(ETH_TAG, "GPIO ISR handler has been already installed");
    //     ret = ESP_OK; // ISR handler has been already installed so no issues
    // } else {
    //     ESP_LOGE(ETH_TAG, "GPIO ISR handler install failed");
    //     return ret;
    // }

    // // Init SPI bus
    // spi_bus_config_t buscfg = {
    //     .mosi_io_num = SPI_MOSI,
    //     .miso_io_num = SPI_MISO,
    //     .sclk_io_num = SPI_CLK,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    //     .max_transfer_sz = ETH_MAX_BYTES
    // };
    
    // ret = spi_bus_initialize(ETH_HOST, &buscfg, SPI_DMA_CH_AUTO);

    return ret;
}

// esp_eth_handle_t eth_init_spi(spi_eth_module_config_t *spi_eth_module_config, esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out)
// {
//     esp_eth_handle_t ret = NULL;

//     // Init common MAC and PHY configs to default
//     eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
//     eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

//     // Update PHY config based on board specific configuration
//     phy_config.phy_addr = spi_eth_module_config->phy_addr;
//     phy_config.reset_gpio_num = spi_eth_module_config->phy_reset_gpio;

//     // Configure SPI interface for specific SPI module
//     spi_device_interface_config_t spi_devcfg = {
//         .mode = 0,
//         .clock_speed_hz = ETH_SPI_CLOCK_MHZ * 1000 * 1000,
//         .spics_io_num = spi_eth_module_config->spi_cs_gpio,
//         .queue_size = 20
//     };

//     // Init vendor specific MAC config to default, and create new SPI Ethernet MAC instance
//     // and new PHY instance based on board configuration
//     eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(ETH_HOST, &spi_devcfg);
//     w5500_config.int_gpio_num = spi_eth_module_config->int_gpio;
//     w5500_config.poll_period_ms = spi_eth_module_config->polling_ms;
//     esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
//     esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

//     // Init Ethernet driver to default and install it
//     esp_eth_handle_t eth_handle = NULL;
//     esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);
//     if ( esp_eth_driver_install(&eth_config_spi, &eth_handle) != ESP_OK )
//     {
//         ESP_LOGE(ETH_TAG, "SPI Ethernet driver install failed");
//         return NULL;
//     }

//     // The SPI Ethernet module might not have a burned factory MAC address, we can set it manually.
//     if (spi_eth_module_config->mac_addr != NULL) {
//         // ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, spi_eth_module_config->mac_addr) == ESP_OK,
//         //                                 NULL, err, TAG, "SPI Ethernet MAC address config failed");
//     }

//     if (mac_out != NULL) {
//         *mac_out = mac;
//     }
//     if (phy_out != NULL) {
//         *phy_out = phy;
//     }
//     return eth_handle;
// // err:
// //     if (eth_handle != NULL) {
// //         esp_eth_driver_uninstall(eth_handle);
// //     }
// //     if (mac != NULL) {
// //         mac->del(mac);
// //     }
// //     if (phy != NULL) {
// //         phy->del(phy);
// //     }
// //     return ret;
// }
