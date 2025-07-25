// threshold_manager.c

#include "threshold_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define NVS_NAMESPACE "config"
#define NVS_KEY_UMBRAL "umbral"

esp_err_t threshold_set(float valor) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, NVS_KEY_UMBRAL, &valor, sizeof(float));
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}

esp_err_t threshold_get(float *valor) {
    size_t required_size = sizeof(float);
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    err = nvs_get_blob(handle, NVS_KEY_UMBRAL, valor, &required_size);
    nvs_close(handle);
    return err;
}

esp_err_t threshold_init_with_default(float *valor, float DEFAULT_THRESHOLD) {
    esp_err_t err = threshold_get(valor);
    if (err == ESP_OK) {
        ESP_LOGI("UMBRAL", "Umbral cargado desde NVS: %.2f", *valor);
        return ESP_OK;
    }

    // Si no se encontró, establecer por defecto
    *valor = DEFAULT_THRESHOLD;
    ESP_LOGW("UMBRAL", "Umbral no encontrado. Usando valor por defecto: %.2f", *valor);
    return threshold_set(*valor);
}
