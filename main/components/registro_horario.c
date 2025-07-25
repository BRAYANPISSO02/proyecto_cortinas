#include "registro_horario.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"

#define TAG "REGISTRO_HORARIO"
#define NAMESPACE "registros"

void registro_horario_init(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_close(handle);
        ESP_LOGI(TAG, "Inicializado correctamente.");
    } else {
        ESP_LOGE(TAG, "Error inicializando NVS.");
    }
}

bool registro_horario_guardar(const char *registro_str) {
    nvs_handle_t handle;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;

    for (int i = 0; i < MAX_REGISTROS; i++) {
        char key[8];
        snprintf(key, sizeof(key), "r%d", i);
        size_t required_size = 0;
        esp_err_t err = nvs_get_str(handle, key, NULL, &required_size);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            nvs_set_str(handle, key, registro_str);
            nvs_commit(handle);
            nvs_close(handle);
            return true;
        }
    }

    nvs_close(handle);
    return false;
}

int registro_horario_listar(char registros[][LONGITUD_REGISTRO], size_t max_registros) {
    nvs_handle_t handle;
    if (nvs_open(NAMESPACE, NVS_READONLY, &handle) != ESP_OK) return 0;

    int count = 0;
    for (int i = 0; i < MAX_REGISTROS && count < max_registros; i++) {
        char key[8];
        snprintf(key, sizeof(key), "r%d", i);
        size_t len = LONGITUD_REGISTRO;
        esp_err_t err = nvs_get_str(handle, key, registros[count], &len);
        if (err == ESP_OK) {
            count++;
        }
    }

    nvs_close(handle);
    return count;
}

void registro_horario_verificar_y_enviar(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char registros[MAX_REGISTROS][LONGITUD_REGISTRO];
    int total = registro_horario_listar(registros, MAX_REGISTROS);

    char hora_actual[6];
    snprintf(hora_actual, sizeof(hora_actual), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    for (int i = 0; i < total; i++) {
        if (strncmp(registros[i], hora_actual, 5) == 0 && registros[i][5] == ':') {
            int valor = atoi(&registros[i][6]);
            xQueueSend(cola_porcentaje, &valor, 0);
            ESP_LOGI(TAG, "Registro activado %s → enviado: %d", registros[i], valor);
        }
    }
}
