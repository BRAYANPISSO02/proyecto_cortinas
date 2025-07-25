#include "wifi_manager.h"
#include "common.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include <string.h>

static bool ap_ready = false;
static bool sta_auto_reconnect = true;
static int sta_retry_count = 0;
#define MAX_RETRIES 3

static EventGroupHandle_t wifi_event_group;
static bool wifi_connected = false;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG_WIFI, "AP listo");
        ap_ready = true;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (ap_ready) { // Solo conectar STA si el AP ya está listo
            esp_wifi_connect();
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG_WIFI, "Conexión STA perdida (razón: %d)", event->reason);
        
        if (sta_retry_count < MAX_RETRIES && sta_auto_reconnect) {
            sta_retry_count++;
            vTaskDelay(pdMS_TO_TICKS(5000)); // Espera 5 segundos antes de reintentar
            esp_wifi_connect();
        } else {
            ESP_LOGW(TAG_WIFI, "Máximo de reintentos alcanzado");
            sta_auto_reconnect = false;
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
        sta_retry_count = 0; // Resetear contador al conectar
    }
}

void wifi_manager_init(void) 
{
    wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Primero inicializa solo el AP
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Intentar cargar configuración personalizada del AP desde NVS
    char ap_ssid[MAX_SSID_LEN] = {0};
    char ap_pass[MAX_PASS_LEN] = {0};

    esp_err_t ap_loaded = wifi_manager_load_ap_credentials(ap_ssid, ap_pass);

    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = {0},
            .ssid_len = 0,
            .channel = WIFI_AP_CHANNEL,
            .password = {0},
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    if (ap_loaded == ESP_OK) {
        strncpy((char *)wifi_config_ap.ap.ssid, ap_ssid, sizeof(wifi_config_ap.ap.ssid));
        strncpy((char *)wifi_config_ap.ap.password, ap_pass, sizeof(wifi_config_ap.ap.password));
        wifi_config_ap.ap.ssid_len = strlen(ap_ssid);
        ESP_LOGI(TAG_WIFI, "Configuración AP cargada de NVS");
    } else {
        strncpy((char *)wifi_config_ap.ap.ssid, WIFI_AP_SSID, sizeof(wifi_config_ap.ap.ssid));
        strncpy((char *)wifi_config_ap.ap.password, WIFI_AP_PASSWORD, sizeof(wifi_config_ap.ap.password));
        wifi_config_ap.ap.ssid_len = strlen(WIFI_AP_SSID);
        ESP_LOGW(TAG_WIFI, "Usando configuración AP por defecto");
    }

    // Mostrar la configuración actual del AP:
    ESP_LOGI(TAG_WIFI, "AP SSID: %s", wifi_config_ap.ap.ssid);
    ESP_LOGI(TAG_WIFI, "AP Password: %s", wifi_config_ap.ap.password);


    
    // Configuración inicial STA (vacía)
    wifi_config_t wifi_config_sta = {0};
    
    // Establece modo AP primero
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    
    // Registra handlers de eventos
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));   

    // Inicia WiFi (solo AP inicialmente)
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG_WIFI, "AP iniciado, esperando configuración STA...");
}

// Nueva función para iniciar conexión STA manualmente
void wifi_manager_start_sta(void) 
{
    sta_auto_reconnect = true;
    sta_retry_count = 0;
    esp_wifi_connect();
}

esp_err_t wifi_manager_try_connect_saved(void)
{
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASS_LEN];
    
    if (wifi_manager_load_credentials(ssid, password) != ESP_OK) {
        ESP_LOGW(TAG_WIFI, "No hay credenciales STA guardadas");
        return ESP_FAIL;
    }

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    // Inicia la conexión STA
    wifi_manager_start_sta();
    
    ESP_LOGI(TAG_WIFI, "Intentando conexión STA con SSID: %s", ssid);
    return ESP_OK;
}

esp_err_t wifi_manager_save_credentials(const char* ssid, const char* password)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi_creds", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "pass", password));
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
    ESP_LOGI(TAG_WIFI, "Credenciales WiFi guardadas");
    return ESP_OK;
}

esp_err_t wifi_manager_load_credentials(char* ssid, char* password)
{
    size_t ssid_len = MAX_SSID_LEN;
    size_t pass_len = MAX_PWD_LEN;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len);
    if (err != ESP_OK) return err;

    err = nvs_get_str(nvs_handle, "pass", password, &pass_len);
    if (err != ESP_OK) return err;

    nvs_close(nvs_handle);
    return ESP_OK;
}

bool wifi_manager_is_connected(void)
{
    return wifi_connected;
}

void wifi_manager_restart_device(void)
{
    ESP_LOGI(TAG_WIFI, "Reiniciando para aplicar cambios...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}

// Función para guardar credenciales del AP en NVS
esp_err_t wifi_manager_save_ap_credentials(const char* ssid, const char* password) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi_ap_creds", NVS_READWRITE, &nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ap_ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ap_pass", password));
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
    ESP_LOGI(TAG_WIFI, "Credenciales AP guardadas");
    return ESP_OK;
}

// Función para cargar credenciales del AP desde NVS
esp_err_t wifi_manager_load_ap_credentials(char* ssid, char* password) {
    size_t ssid_len = MAX_SSID_LEN;
    size_t pass_len = MAX_PASS_LEN;

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_ap_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    err = nvs_get_str(nvs_handle, "ap_ssid", ssid, &ssid_len);
    if (err != ESP_OK) return err;

    err = nvs_get_str(nvs_handle, "ap_pass", password, &pass_len);
    if (err != ESP_OK) return err;

    nvs_close(nvs_handle);
    return ESP_OK;
}

// Función para actualizar la configuración del AP
esp_err_t wifi_manager_update_ap_config(const char* ssid, const char* password) {
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = {0},
            .ssid_len = strlen(ssid),
            .channel = WIFI_AP_CHANNEL,
            .password = {0},
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    
    strncpy((char*)wifi_config_ap.ap.ssid, ssid, sizeof(wifi_config_ap.ap.ssid));
    strncpy((char*)wifi_config_ap.ap.password, password, sizeof(wifi_config_ap.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    
    // Guardar las nuevas credenciales en NVS
    ESP_ERROR_CHECK(wifi_manager_save_ap_credentials(ssid, password));
    
    ESP_LOGI(TAG_WIFI, "Configuración AP actualizada: SSID=%s, PSWD=%s", ssid, password);
    return ESP_OK;
}

esp_err_t wifi_manager_reset_ap_config(void) {
    // Restaurar configuración predeterminada
    wifi_config_t default_ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASSWORD,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &default_ap_config));

    // Guardar valores por defecto en NVS
    return wifi_manager_save_ap_credentials(WIFI_AP_SSID, WIFI_AP_PASSWORD);
}

esp_err_t wifi_manager_clear_sta_credentials(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;

    nvs_erase_key(nvs_handle, "ssid");
    nvs_erase_key(nvs_handle, "pass");

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    ESP_LOGI(TAG_WIFI, "Credenciales STA eliminadas (modo por defecto)");
    return err;
}
