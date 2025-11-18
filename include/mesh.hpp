#ifndef MESH_CONTROLLER
#define MESH_CONTROLLER

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_mesh.h"
#include "sdkconfig.h"

#include "string"
#include "format"

#include "mqtt_app.hpp"
#include "indicator.hpp"

#include "pb_encode.h"
#include "pb_decode.h"
#include "mqtt.pb.h"

/*******************************************************
 *                Constants
 *******************************************************/
#define RX_SIZE     (1500)
#define TX_SIZE     (1460)

// Mesh Configurations
static const char* ROUTER_SSID = "RCS_Mesh";
static const char* ROUTER_PASSWD = "RCSMESH_PASS";
static const char* MESH_PASSWD = "MAP_PASSWD";
static const uint8_t MESH_ID[6] = { 3, 0, 9, 9, 4, 1 }; // Shared ID of all MESH nodes: 309941
static const uint8_t MESH_CHANNEL = 0; // 1-13 to manually define, 0 to auto-select.
static const uint8_t MAX_CONNECTIONS = 4; // Mesh Stations (Root + 0-3 Root enabled)
static const uint8_t NONMESH_MAX_CONNECTIONS = 0; // NON-Mesh Stations
static const uint8_t MESH_ROUTE_TABLE_MAX_SIZE = 50; // Max Routing table size
static const mesh_addr_t BROADCAST_ADDR = { .addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
static const char MESH_TAG[] = "Mesh Controller";
static const char MESH_RX_TAG[] = "MESH_RX";
static const char MESH_TX_TAG[] = "MESH_TX";


/*******************************************************
 *                Dynamic Variables
 *******************************************************/

// Network Variables
static esp_netif_t* netif_sta = NULL;
// static esp_ip4_addr_t s_current_ip;
static int s_route_table_size = 0;
// static mesh_addr_t s_route_table[CONFIG_MESH_ROUTE_TABLE_SIZE] = { 0 };
static SemaphoreHandle_t route_table_lock = NULL;

// Mesh Variables
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_running = true; // used in all task while loops
static bool is_mesh_connected = false;
static mesh_addr_t mesh_root_addr;
static mesh_addr_t mesh_parent_addr;
static mesh_addr_t mesh_current_addr;
static int mesh_layer = -1;

static SemaphoreHandle_t rx_led = NULL;

/*******************************************************
 *                Function Declarations
 *******************************************************/
void start_mesh();
void initialize_wifi();

esp_err_t mesh_send(const mesh_addr_t* dest_addr, const mesh_data_t* data);
esp_err_t mesh_broadcast(const mesh_data_t* data);
esp_err_t mesh_send_root(const mesh_data_t* data);

// #define MESH_SEND_TO_ROOT(data) do { mesh_send(&mesh_root_addr, data) } while(0) 
// #define MESH_BROADCAST(data) do { mesh_send(&mesh_root_addr, data) } while(0) 

#endif