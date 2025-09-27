#ifndef MESH_CONTROLLER
#define MESH_CONTROLLER

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_mesh.h"

namespace mesh_controller
{
    /*******************************************************
     *                Constants
     *******************************************************/
    #define RX_SIZE     (1500)
    #define TX_SIZE     (1460)
    #define TOPOLOGY MESH_TOPO_TREE
    #define MAX_LAYER 8

    #define ROOT_NODE 1

    static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77}; // Shared ID of all MESH nodes
    static const uint8_t MESH_CHANNEL = 0; // 1-13 to manually define, 0 to auto-select.
    static const char* MESH_ROUTER_SSID = "RCS_Mesh";
    static const char* MESH_ROUTER_PASSWD = "RCSMESH_PASS";
    static const char* MESH_AP_PASSWD = "MAP_PASSWD";

    static const uint8_t MAX_CONNECTIONS = 6; // Mesh Stations
    static const uint8_t NONMESH_MAX_CONNECTIONS = 0; // NON-Mesh Stations


    static const char* MESH_TAG = "Mesh Controller";
    static uint8_t tx_buf[TX_SIZE] = { 0, };
    static uint8_t rx_buf[RX_SIZE] = { 0, };
    static bool is_running = true;
    static bool is_mesh_connected = false;
    static mesh_addr_t mesh_parent_addr;
    static int mesh_layer = -1;
    static esp_netif_t *netif_sta = NULL;

    /*******************************************************
     *                Function Declarations
     *******************************************************/
    void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
    void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);

    void start_mesh();
}

#endif