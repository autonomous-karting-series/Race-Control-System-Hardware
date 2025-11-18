#include "mesh.hpp"

esp_err_t mesh_send(const mesh_addr_t* dest_addr, const mesh_data_t* data)
{
    // Could optimize broadcasting only to leaf node groups using group ids.
    if (memcmp(dest_addr, BROADCAST_ADDR.addr, 6) == 0) {
        ESP_LOGI(MESH_TAG, "Broadcasting!");

        int route_table_size = 0;
        mesh_addr_t route_table[MESH_ROUTE_TABLE_MAX_SIZE];
        esp_mesh_get_routing_table((mesh_addr_t*)&route_table, MESH_ROUTE_TABLE_MAX_SIZE * 6, &route_table_size);

        for (int i = 0; i < route_table_size; i++) {
            ESP_LOGI(MESH_TAG, "Broadcast: Sending to [%d] " MACSTR, i, MAC2STR(route_table[i].addr));
            esp_err_t err = esp_mesh_send(&route_table[i], data, MESH_DATA_P2P, NULL, 0);
            if (ESP_OK != err) {
                ESP_LOGE(MESH_TAG, "Send with err code %d %s", err, esp_err_to_name(err));
            }
        }

    } else {
        // Standard P2P
        esp_err_t err = esp_mesh_send(dest_addr, data, MESH_DATA_P2P, NULL, 0);
        ESP_LOGI(MESH_TAG, "P2P: Sending to " MACSTR, MAC2STR(dest_addr->addr));
        if (err != ESP_OK) {
            ESP_LOGE(MESH_TAG, "Send with err code %d %s", err, esp_err_to_name(err));
            return err;
        }
    }

    return ESP_OK;
}

esp_err_t mesh_send_root(const mesh_data_t* data)
{
    esp_err_t err = mesh_send(&mesh_root_addr, data);
    return err;
}

esp_err_t mesh_broadcast(const mesh_data_t* data)
{
    esp_err_t err = mesh_send(&BROADCAST_ADDR, data);
    return err;
}

/// @brief Task for receiving, decoding, and forwarding messages transmitted over the Mesh Network.
/// @param arg 
void esp_mesh_rx_main(void *arg)
{
    is_running = true;
    
    MQTTMessage recv_msg;
    mesh_addr_t from;
    esp_err_t err;
    int flag = 0;
    int recv_count = 0;

    mesh_data_t data;
    data.data = rx_buf;
    data.size = RX_SIZE;

    while (is_running)
    {
        data.size = RX_SIZE;
        recv_msg = MQTTMessage_init_default;
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        notify_rx_led();

        // Skip if receive has error or data is empty
        if (err != ESP_OK || !data.size)
        {
            ESP_LOGE(MESH_RX_TAG, "err:0x%x, size:%d", err, data.size);
            continue;
        }

        // Ignore broadcasts from self
        if (!memcmp(&from, &mesh_current_addr, sizeof(from)))
        {
            ESP_LOGD(MESH_RX_TAG, "Received message from self, ignoring.");
            continue;
        }

        // Increment recv and decode message
        recv_count++;
        if (!(recv_count % 1))
        {
            pb_istream_t pb_istream = pb_istream_from_buffer(data.data, data.size);
            if (!pb_decode(&pb_istream, &MQTTMessage_msg, &recv_msg))
            {
                ESP_LOGE(MESH_RX_TAG, "NanoPB Failed to decode with error: %s", pb_istream.errmsg);
                continue;
            }
            
            if (esp_mesh_is_root())
            {
                if (recv_msg.topic == MQTTMessage_Topic_MQTT_TOPIC_KART)
                {
                    // /kart/##:##:##:##:##:##
                    std::string topic_str = std::format("/kart/{:x}:{:x}:{:x}:{:x}:{:x}:{:x}", MAC2STR(from.addr));
                    MQTT_AppPublish(topic_str.c_str(), (char *)data.data);
                }
            }
            else
            {
                switch (recv_msg.topic)
                {
                case MQTTMessage_Topic_MQTT_TOPIC_KART:
                    ESP_LOGI(MESH_RX_TAG, "KART TOPIC");
                    break;
                case MQTTMessage_Topic_MQTT_TOPIC_TRACK:
                {
                    ESP_LOGI(MESH_RX_TAG, "TRACK TOPIC");

                    MQTTMessage kart_msg = MQTTMessage_init_zero;
                    kart_msg.topic = MQTTMessage_Topic_MQTT_TOPIC_KART;
                    kart_msg.which_payload = MQTTMessage_kart_status_tag;
                    kart_msg.payload.kart_status =
                        {
                            .is_kart_connected = true,
                            .kart_id = 12,
                            .control = Kart_ControlMode_CONTROL_AUTO,
                            .speed = 3.2,
                            .steering_angle = 0.2343,
                            .has_position = true,
                            .position = {
                                .latitude = 50.235432423,
                                .longitude = 23.2332132
                            }
                        };

                    pb_ostream_t ostream = pb_ostream_from_buffer(tx_buf, sizeof(tx_buf));
                    if (!pb_encode(&ostream, &MQTTMessage_msg, &kart_msg))
                    {
                        ESP_LOGE(MESH_RX_TAG, "NanoPB Failed to encode with error: %s", ostream.errmsg);
                    }

                    ESP_LOGI(MESH_RX_TAG, "NanoPB OStream bytes written: %d", ostream.bytes_written);
                    if (ostream.bytes_written != 0)
                    {
                        mesh_data_t data;
                        data.data = tx_buf;
                        data.size = ostream.bytes_written;
                        data.proto = MESH_PROTO_MQTT;
                        data.tos = MESH_TOS_P2P;
                        mesh_send_root(&data);
                    }
                }
                break;
                default:
                    ESP_LOGI(MESH_RX_TAG, "UNDEFINED");
                    break;
                }
            }

// #if true
//             ESP_LOGW(MESH_TAG,
//                     "[#RX:%d][L:%d] parent:" MACSTR ", receive from " MACSTR ", size:%d, heap:%" PRId32 ", flag:%d[err:0x%x, proto:%d, tos:%d]",
//                     recv_count, mesh_layer,
//                     MAC2STR(mesh_parent_addr.addr), MAC2STR(from.addr),
//                     data.size, esp_get_minimum_free_heap_size(), flag, err, data.proto,
//                     data.tos);
// #endif

        }
    }
    vTaskDelete(NULL);
}

esp_err_t esp_mesh_comm_start(void)
{
    static bool is_comm_started = false;
    if (!is_comm_started)
    {
        is_comm_started = true;
        xTaskCreate(esp_mesh_rx_main, "MPRX", 3072, NULL, 5, NULL);
    }
    return ESP_OK;
}

void mesh_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_addr_t id = {
        0,
    };
    static uint16_t last_layer = 0;

    switch (event_id)
    {
    case MESH_EVENT_STARTED:
    {
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:" MACSTR "", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED:
    {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_CHILD_CONNECTED:
    {
        mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, " MACSTR "",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));
    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED:
    {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, " MACSTR "",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_PARENT_CONNECTED:
    {
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:" MACSTR "%s, ID:" MACSTR ", duty:%d",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" : (mesh_layer == 2) ? "<layer2>"
                                                                   : "",
                 MAC2STR(id.addr), connected->duty);
        last_layer = mesh_layer;
        is_mesh_connected = true;
        if (esp_mesh_is_root())
        {
            if (netif_sta)
            {
                esp_netif_dhcpc_stop(netif_sta);
            }
            else
            {
                ESP_LOGE("MESH_NET", "Netif_STA is null");
            }
            esp_netif_dhcpc_start(netif_sta);
        }
        esp_mesh_comm_start();
    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED:
    {
        mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 disconnected->reason);
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE:
    {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" : (mesh_layer == 2) ? "<layer2>"
                                                                   : "");
        last_layer = mesh_layer;
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS:
    {
        mesh_root_addr = *((mesh_event_root_address_t *) event_data);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:" MACSTR "",
                 MAC2STR(mesh_root_addr.addr));
    }
    break;
    case MESH_EVENT_TODS_STATE:
    {
        mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);

        // Leaf can now send heartbeats.
        if ( esp_mesh_get_type() == MESH_LEAF )
        {
            if (*toDs_state)
            {
                // Can reach to external network -- start communications
            }
            else
            {
                // Cannot reach external network -- command controlled stops
            }
        }
    }
    break;
    default:
        ESP_LOGI(MESH_TAG, "unknown id:%" PRId32 "", event_id);
        break;
    }
}

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGW("IP_EVENT", "<IP_EVENT_STA_GOT_IP> | IP:" IPSTR, IP2STR(&event->ip_info.ip));

    if (esp_mesh_is_root())
    {
        esp_mesh_post_toDS_state(true);
    }

    route_table_lock = xSemaphoreCreateMutex();
    esp_mesh_comm_mqtt_task_start();
}

void initialize_wifi()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Run esp_netif_init() only ONCE on startup
    ESP_ERROR_CHECK(esp_netif_init());

    // Attaches default netif station
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL));
    assert(netif_sta);

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_get_mac(WIFI_IF_STA, mesh_current_addr.addr);
    ESP_LOGW("WIFI", "MAC ADDR: " MACSTR, MAC2STR(mesh_current_addr.addr));
}

void start_mesh()
{
    ESP_ERROR_CHECK(esp_mesh_init());

    ESP_ERROR_CHECK(esp_mesh_set_topology(MESH_TOPO_TREE));
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(8));
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(128));
    ESP_ERROR_CHECK(esp_mesh_disable_ps());
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(10));

    /**
     * Mesh Config Creation
     */
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    cfg.channel = MESH_CHANNEL;
    memcpy((uint8_t *)&cfg.mesh_id, MESH_ID, 6);

    // SoftAP Config - connect Mesh Network
    cfg.mesh_ap.max_connection = MAX_CONNECTIONS;
    memcpy((uint8_t *)&cfg.mesh_ap.password, MESH_PASSWD, strlen(MESH_PASSWD));

    // Station Config - connect Root to Router
    cfg.router.ssid_len = strlen(ROUTER_SSID);
    memcpy((uint8_t *)&cfg.router.ssid, ROUTER_SSID, cfg.router.ssid_len);
    memcpy((uint8_t *)&cfg.router.password, ROUTER_PASSWD, strlen(ROUTER_PASSWD));

    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));

    #if CONFIG_ROOT_NODE
        esp_mesh_set_type(MESH_ROOT);
    #else
        esp_mesh_set_type(MESH_LEAF);
    #endif

    esp_mesh_fix_root(true);

    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));

    ESP_ERROR_CHECK(esp_mesh_start());

    ESP_LOGI(MESH_TAG, "mesh starts successfully, heap:%" PRId32 ", %s<%d>%s, ps:%d", esp_get_minimum_free_heap_size(),
             esp_mesh_is_root_fixed() ? "root fixed" : "root not fixed",
             esp_mesh_get_topology(), esp_mesh_get_topology() ? "(chain)" : "(tree)", esp_mesh_is_ps_enabled());
}
