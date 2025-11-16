#include "mqtt_app.hpp"

static esp_err_t MQTT_EventProcess(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
            is_mqtt_connected = true;
            if (esp_mqtt_client_subscribe(mqtt_handle, MQTT_TRACK_TOPIC, 0) < 0)
            {
                // Disconnect to retry the subscribe after auto-reconnect timeout
                esp_mqtt_client_disconnect(mqtt_handle);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            is_mqtt_connected = false;
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(MQTT_TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(MQTT_TAG, "DATA=%.*s", event->data_len, event->data);

            mesh_data_t data;
            data.data = (uint8_t*) event->data;
            data.size = event->data_len;
            data.proto = MESH_PROTO_MQTT;
            data.tos = MESH_TOS_P2P;

            mesh_broadcast(&data);
        }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void MQTT_EventCb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%d", event_base, event_id);
    MQTT_EventProcess((esp_mqtt_event_handle_t) event_data);
}

void MQTT_AppPublish(const char* pTopic, const char* pPublishString)
{
    if (mqtt_handle)
    {
        int msg_id = esp_mqtt_client_publish(mqtt_handle, pTopic, pPublishString, 0, 1, 0);
        ESP_LOGI(MQTT_TAG, "sent publish returned msg_id=%d", msg_id);
    }
}

void mqtt_app_start(void)
{
    static esp_mqtt_client_config_t MQTT_config = {
        .broker = {
            .address ={
                .uri = "mqtt://192.168.1.197",
                .port = 1883
            }
        }
    };

    mqtt_handle = esp_mqtt_client_init(&MQTT_config);

    esp_mqtt_client_register_event(mqtt_handle, MQTT_EVENT_ANY, MQTT_EventCb, mqtt_handle);
    esp_mqtt_client_start(mqtt_handle);
}

void esp_mesh_mqtt_task(void *arg)
{
    is_running = true;
    char *print;
    mesh_data_t data;
    esp_err_t err;

    mqtt_app_start();
    while (is_running) {

        if(!is_mqtt_connected)
        {
            esp_mqtt_client_reconnect(mqtt_handle);
        }

        vTaskDelay(2 * 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

esp_err_t esp_mesh_comm_mqtt_task_start()
{
    static bool is_comm_mqtt_task_started = false;
    ESP_LOGI("TASK Started", "Started MQTT Tasks");

    if (!is_comm_mqtt_task_started) {
        xTaskCreate(esp_mesh_mqtt_task, "mqtt task", 3072, NULL, 5, NULL);
        is_comm_mqtt_task_started = true;
    }
    return ESP_OK;
}
