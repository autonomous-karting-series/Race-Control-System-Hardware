#ifndef MQTT_APP
#define MQTT_APP

#include "esp_log.h"
#include "unordered_map"
#include "mqtt_client.h"

#include "mesh.hpp"

#include "pb_encode.h"
#include "pb_decode.h"
#include "mqtt.pb.h"

/*******************************************************
 *                Constants
 *******************************************************/

#define MQTT_TRACK_TOPIC "/track"

static const char *MQTT_BROKER_URI = "192.168.1.197";
static const char *MQTT_TAG = "MQTT Client";

/*******************************************************
 *                Dynamic Variables
 *******************************************************/

static esp_mqtt_client_handle_t mqtt_handle = NULL;
static bool is_mqtt_connected = false;
static int32_t kart_id = -1;

/*******************************************************
 *                Function Declarations
 *******************************************************/

void mqtt_app_start();
void MQTT_AppPublish(const char *pTopic, const char *pPublishString);

esp_err_t esp_mesh_comm_mqtt_task_start();


#endif
