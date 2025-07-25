#include "http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include <sys/param.h>
#include "esp_spiffs.h"
#include "common.h"
#include "wifi_manager.h"
#include "threshold_manager.h"
#include "time_manager.h"
#include "sensors_adc.h"
#include "operation_mode.h"
#include "registro_horario.h"
#include "esp_sntp.h"
#include "cJSON.h"
#include <inttypes.h>

static httpd_handle_t server = NULL;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t styles_css_start[] asm("_binary_styles_css_start");
extern const uint8_t styles_css_end[]   asm("_binary_styles_css_end");
extern const uint8_t script_js_start[]  asm("_binary_script_js_start");
extern const uint8_t script_js_end[]    asm("_binary_script_js_end");

// ==========================================
// WEB PAGE
// ==========================================

esp_err_t root_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "Solicitado / (index.html)");
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
}

esp_err_t css_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "Solicitado / (styles.css)");
    httpd_resp_set_type(req, "text/css");
    return httpd_resp_send(req, (const char *)styles_css_start, styles_css_end - styles_css_start);
}

esp_err_t js_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "Solicitado / (script.js)");
    httpd_resp_set_type(req, "application/javascript");
    return httpd_resp_send(req, (const char *)script_js_start, script_js_end - script_js_start);
}

// ==========================================
// TIPO GET
// ==========================================

// /api/get_threshold
esp_err_t get_threshold_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/get_threshold] Petición recibida");
    float valor;
    if (threshold_get(&valor) != ESP_OK) {
        ESP_LOGE(TAG_HTTP, "Fallo al obtener umbral desde threshold_manager");
        return httpd_resp_send_500(req);
    }
    char respuesta[64];
    snprintf(respuesta, sizeof(respuesta), "{\"umbral\": %.2f}", valor);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, respuesta);
}

// /api/get_temperatura
esp_err_t get_temp_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/get_temperatura] Petición recibida");
    float temp = 0;
    if (xQueueReceive(cola_temperatura, &temp, 0) == pdTRUE)
    {
        char json_response[64];
        snprintf(json_response, sizeof(json_response), "{\"temperature\": %.2f}", temp);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, json_response);
    }
    else
    {
        ESP_LOGW("HTTP", "No se pudo leer temperatura de la cola");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Fallo al obtener temperatura");
    }
    return ESP_OK;
}

// /api/get_porcentaje_servo
esp_err_t get_porcentaje_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/get_porcentaje_servo] Petición recibida");
    uint8_t pot = 0;
    if (xQueueReceive(cola_porcentaje, &pot, 0) == pdTRUE)
    {
        char json_response[64];
        snprintf(json_response, sizeof(json_response), "{\"potentiometer\": %d}", pot);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, json_response);
    }
    else
    {
        ESP_LOGW("HTTP", "No se pudo leer porcentaje del servo de la cola");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Fallo al obtener valor del potenciómetro");
    }
    return ESP_OK;
}

// /api/get_ap_config
esp_err_t get_ap_config_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/get_ap_config] Petición recibida");
    char ap_ssid[MAX_SSID_LEN] = {0};
    char ap_password[MAX_PASS_LEN] = {0};

    if (wifi_manager_load_ap_credentials(ap_ssid, ap_password) != ESP_OK) {
        ESP_LOGE(TAG_HTTP, "Error al leer configuración del AP desde wifi_manager");
        return httpd_resp_send_500(req);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ap_ssid", ap_ssid);
    cJSON_AddStringToObject(root, "ap_password", ap_password);

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    cJSON_Delete(root);
    free((void *)json_str);
    return ESP_OK;
}

// GET /api/get_mode.json
    // Tabla de nombres de modos
const char* mode_names[] = {
    "Potenciómetro",
    "Temperatura",
    "Registros"
};

esp_err_t get_mode_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/get_mode] Petición recibida");
    selector current = operation_mode_get();

    char json[64];
    snprintf(json, sizeof(json),
             "{\"mode\": %d, \"name\": \"%s\"}",
             current,
             (current >= 0 && current < 3) ? mode_names[current] : "Desconocido");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


// ==========================================
// TIPO SET
// ==========================================

// /api/set_threshold
esp_err_t set_threshold_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/set_threshold] Petición recibida");
    char buffer[100];
    int len = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (len <= 0) {
        ESP_LOGE(TAG_HTTP, "Error al recibir datos de threshold");
        return httpd_resp_send_500(req);
    }
    buffer[len] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        ESP_LOGE(TAG_HTTP, "JSON inválido en set_threshold");
        return httpd_resp_send_500(req);
    }

    cJSON *umbral = cJSON_GetObjectItem(json, "umbral");
    if (!cJSON_IsNumber(umbral)) {
        ESP_LOGE(TAG_HTTP, "Campo 'umbral' inválido o ausente");
        cJSON_Delete(json);
        return httpd_resp_send_500(req);
    }

    float valor = umbral->valuedouble;
    if (threshold_set(valor) != ESP_OK) {
        ESP_LOGE(TAG_HTTP, "Error al guardar nuevo umbral");
        cJSON_Delete(json);
        return httpd_resp_send_500(req);
    }

    cJSON_Delete(json);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"status\": \"ok\"}");
}

// /api/set_ap_config
esp_err_t set_ap_config_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/set_ap_config] Petición recibida");
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        return ESP_FAIL;
    }

    buf[ret] = 0;

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        return ESP_FAIL;
    }

    const cJSON *ssid = cJSON_GetObjectItem(json, "ap_ssid");
    const cJSON *pass = cJSON_GetObjectItem(json, "ap_password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(pass)) {
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    // Aquí guardas en NVS
    if (wifi_manager_load_ap_credentials(ssid->valuestring, pass->valuestring) != ESP_OK) {
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    cJSON *res = cJSON_CreateObject();
    cJSON_AddStringToObject(res, "status", "ok");

    const char *resp_str = cJSON_Print(res);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str, strlen(resp_str));

    free((void *)resp_str);
    cJSON_Delete(json);
    cJSON_Delete(res);

    return ESP_OK;
}

// - /api/set_wifi
esp_err_t set_wifi_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/set_wifi] Petición recibida");
    char buffer[150];
    int len = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (len <= 0) return httpd_resp_send_500(req);
    buffer[len] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    if (!json) return httpd_resp_send_500(req);

    cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    cJSON *pass = cJSON_GetObjectItem(json, "password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(pass)) {
        cJSON_Delete(json);
        ESP_LOGE(TAG_HTTP, "Error: credenciales inválidad");
        return httpd_resp_send_500(req);
    }

    // Guardar en NVS
    wifi_manager_save_credentials(ssid->valuestring, pass->valuestring);

    // Intentar conectar
    wifi_manager_try_connect_saved();

    cJSON_Delete(json);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"status\": \"ok\"}");
}


// /api/reset_ap
esp_err_t reset_ap_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/reset_ap] Petición recibida");
    if (wifi_manager_reset_ap_config() == ESP_OK) {
        httpd_resp_sendstr(req, "AP restaurado a valores por defecto");
    } else {
        ESP_LOGE(TAG_HTTP, "Fallo al reiniciar AP");
        return httpd_resp_send_500(req);
    }
    return ESP_OK;
}

// /api/reset_sta
esp_err_t reset_sta_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/reset_sta] Petición recibida");
    if (wifi_manager_clear_sta_credentials() == ESP_OK) {
        httpd_resp_sendstr(req, "Credenciales STA eliminadas");
    } else {
        ESP_LOGE(TAG_HTTP, "Fallo al reiniciar STA");
        return httpd_resp_send_500(req);      
    }
    return ESP_OK;
}

// /api/set_mode
esp_err_t set_mode_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_HTTP, "[/api/set_mode] Petición recibida");
    char buf[16];
    int ret = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf)-1));
    if (ret <= 0) {
        ESP_LOGE(TAG_HTTP, "Fallo al obtener el modo de operación");
        return httpd_resp_send_500(req);  
    }
    buf[ret] = '\0';
    int new_mode = atoi(buf);
    operation_mode_set((selector)new_mode);

    char response[32];
    snprintf(response, sizeof(response), "Modo cambiado a %d", new_mode);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void http_server_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG_HTTP, "Servidor HTTP iniciado correctamente");

        //  WEB PAGE
        // ==========================
        
        // Página principal
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        // CSS
        httpd_uri_t css_uri = {
            .uri       = "/styles.css",
            .method    = HTTP_GET,
            .handler   = css_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &css_uri);

        // JS
        httpd_uri_t js_uri = {
            .uri       = "/script.js",
            .method    = HTTP_GET,
            .handler   = js_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &js_uri);
        
        //  Tipo GET
        // ==========================

        // Umbral
        httpd_uri_t get_umbral_uri = {
            .uri       = "/api/get_threshold.json",
            .method    = HTTP_GET,
            .handler   = get_threshold_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_umbral_uri);

        // Configuracion AP
        httpd_uri_t get_ap_config_uri = {
            .uri       = "/api/get_ap_config",
            .method    = HTTP_GET,
            .handler   = get_ap_config_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_ap_config_uri);

        // Temperatura medida
        httpd_uri_t get_temp_uri = {
            .uri       = "/api/get_temperatura",
            .method    = HTTP_GET,
            .handler   = get_temp_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_temp_uri);

        // Porcentaje servo
        httpd_uri_t get_porcentaje_uri = {
            .uri       = "/api/get_porcentaje_servo",
            .method    = HTTP_GET,
            .handler   = get_porcentaje_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_porcentaje_uri);


        // Modo de operación
        httpd_uri_t get_mode_uri = {
            .uri       = "/api/get_mode.json",
            .method    = HTTP_GET,
            .handler   = get_mode_handler,
            .user_ctx  = NULL
        };;
        httpd_register_uri_handler(server, &get_mode_uri);

        //  Tipo SET
        // ==========================

        // Umbral
        httpd_uri_t set_umbral_uri = {
            .uri       = "/api/set_threshold.json",
            .method    = HTTP_POST,
            .handler   = set_threshold_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &set_umbral_uri);

        // Configuración AP
        httpd_uri_t set_ap_config_uri = {
            .uri       = "/api/set_ap_config",
            .method    = HTTP_POST,
            .handler   = set_ap_config_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &set_ap_config_uri);


        // Valores por defecto AP
        httpd_uri_t reset_ap_uri = {
            .uri      = "/api/reset_ap",
            .method   = HTTP_POST,
            .handler  = reset_ap_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &reset_ap_uri);

        // Borrar registros STA
        httpd_uri_t reset_sta_uri = {
            .uri      = "/api/reset_sta",
            .method   = HTTP_POST,
            .handler  = reset_sta_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &reset_sta_uri);

        // Wifi
        httpd_uri_t set_wifi_uri = {
            .uri       = "/api/set_wifi.json",
            .method    = HTTP_POST,
            .handler   = set_wifi_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &set_wifi_uri);

        // Cambiar modo de operación
        httpd_uri_t set_mode_uri = {
            .uri       = "/api/set_mode",
            .method    = HTTP_POST,
            .handler   = set_mode_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &set_mode_uri);

        
    } else {
        ESP_LOGE(TAG_HTTP, "Fallo al iniciar el servidor HTTP");
    }
}

void http_server_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG_HTTP, "Servidor HTTP detenido");
    }
}
