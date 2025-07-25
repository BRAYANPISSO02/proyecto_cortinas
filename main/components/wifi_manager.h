#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef struct {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASS_LEN];
} wifi_config_data_t;

// Inicializa WiFi en modo STA+AP
void wifi_manager_init(void);

// Intenta conectar con SSID/pass guardados en NVS
esp_err_t wifi_manager_try_connect_saved(void);

// Guarda nuevas credenciales en NVS
esp_err_t wifi_manager_save_credentials(const char* ssid, const char* password);

// Carga credenciales guardadas en NVS
esp_err_t wifi_manager_load_credentials(char* ssid, char* password);

// Devuelve si está conectado a red WiFi
bool wifi_manager_is_connected(void);

// Reinicia el ESP32 tras nueva configuración
void wifi_manager_restart_device(void);

// Añade estas funciones al header:
esp_err_t wifi_manager_save_ap_credentials(const char* ssid, const char* password);
esp_err_t wifi_manager_load_ap_credentials(char* ssid, char* password);
esp_err_t wifi_manager_update_ap_config(const char* ssid, const char* password);
void wifi_manager_start_sta(void);

esp_err_t wifi_manager_reset_ap_config(void);
esp_err_t wifi_manager_clear_sta_credentials(void);

#endif // WIFI_MANAGER_H