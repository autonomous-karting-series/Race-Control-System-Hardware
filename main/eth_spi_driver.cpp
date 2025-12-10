#include "eth_spi_driver.hpp"

/* Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(ETH_TAG, "Ethernet Got IP Address");
    ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
    ESP_LOGI(ETH_TAG, "IP: " IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(ETH_TAG, "MASK: " IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(ETH_TAG, "GW: " IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
}

void IRAM_ATTR gpio_isr_handler(void* arg)
{  
    /* Disable the Interrupt */
    gpio_intr_disable(START_ETH_GPIO);
    gpio_isr_handler_remove(START_ETH_GPIO);

    xSemaphoreGiveFromISR(eth_start_handle, NULL);

    /* Re-Enable the Interrupt */
    gpio_isr_handler_add(START_ETH_GPIO, gpio_isr_handler, NULL);
    gpio_intr_enable(START_ETH_GPIO);
}

void start_eth_task(void* args)
{
    eth_task_started = true;

    while (true)
    {
        if (xSemaphoreTake(eth_start_handle, portMAX_DELAY) == pdTRUE)
        {
            if (!eth_driver_started)
            {
                ESP_LOGI(ETH_TAG, "Starting Ethernet Driver");
                init_eth();
                eth_driver_started = true;
            }
            else
            {
                ESP_LOGW(ETH_TAG, "Ethernet Driver already started");

                
            }
        }
    }
    vTaskDelete(NULL);
}

void setup_eth_gpio()
{
    eth_start_handle = xSemaphoreCreateBinary();
    xTaskCreate(start_eth_task, "ETH_START", 3072, NULL, 3, NULL);

    gpio_reset_pin(START_ETH_GPIO);
    gpio_input_enable(START_ETH_GPIO);
    gpio_pullup_en(START_ETH_GPIO);

    gpio_set_intr_type(START_ETH_GPIO, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(START_ETH_GPIO, gpio_isr_handler, NULL);
    gpio_intr_enable(START_ETH_GPIO);

    ESP_LOGI(ETH_TAG, "Setup ETH GPIO Successfully");
}

void init_eth()
{
    if (eth_driver_started)
        return;
    
    ESP_LOGI(ETH_TAG, "Init ETH");
    ESP_ERROR_CHECK(ethernet_init_all(&eth_handle, &eth_port_cnt));

    // Create instance(s) of esp-netif for Ethernet(s)
    if (eth_port_cnt == 1) {
        ESP_LOGI(ETH_TAG, "1 ETH found");

        // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
        // default esp-netif configuration parameters.
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        esp_netif_t *eth_netif = esp_netif_new(&cfg);
        // Attach Ethernet driver to TCP/IP stack
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle[0])));
    } else {
        ESP_LOGW(ETH_TAG, "Ethernet Count not expected: %d", eth_port_cnt);
    }
    
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle[0]));

    eth_dev_info_t info = ethernet_init_get_dev_info(eth_handle[0]);
    ESP_LOGI(ETH_TAG, "Device Name: %s", info.name);
    ESP_LOGI(ETH_TAG, "Device type: ETH_DEV_TYPE_SPI(%d)", info.type);
    ESP_LOGI(ETH_TAG, "Pins: cs: %d, intr: %d", info.pin.eth_spi_cs, info.pin.eth_spi_int); 

    eth_driver_started = true;
}

void eth_transmit()
{
    // esp_eth_transmit();
}