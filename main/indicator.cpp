#include "indicator.hpp"

void rx_led_indicator_task(void* arg)
{
    ESP_LOGI(RX_LED_TAG, "Starting LED Task");

    while(true)
    {
        xTaskNotifyWait(0x00,ULONG_MAX, NULL, portMAX_DELAY);
        gpio_set_level(RX_LED, 1);
        vTaskDelay(RX_LED_DELAY / portTICK_PERIOD_MS);
        gpio_set_level(RX_LED, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    rx_led_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t setup_indicator_leds()
{
    gpio_reset_pin(RX_LED);
    gpio_set_direction(RX_LED, GPIO_MODE_OUTPUT);

    return ESP_OK;
}

esp_err_t start_rx_led()
{
    static bool is_rx_led_started = false;

    if (!is_rx_led_started)
    {
        is_rx_led_started = true;
        xTaskCreate(rx_led_indicator_task, "RXLED", 2000, NULL, 3, &rx_led_handle);
    }
    return ESP_OK;
}

esp_err_t notify_rx_led()
{
    if (!rx_led_handle)
    {
        ESP_LOGE(RX_LED_TAG, "RX Handle is Null!");
        return ESP_FAIL;
    }
    
    xTaskNotifyGive(rx_led_handle);
    return ESP_OK;
}