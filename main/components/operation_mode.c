// operation_mode.c
#include "operation_mode.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "common.h"
#include "sensors_adc.h"

#define NVS_NAMESPACE "storage"
#define NVS_KEY "mode"
static const char *TAG = "operation_mode";

selector modo_actual = POTENCIOMETRO;
static TaskHandle_t tarea_actual_handle = NULL;

static void guardar_modo_nvs(selector modo) {
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_u8(handle, NVS_KEY, (uint8_t)modo);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static selector cargar_modo_nvs() {
    nvs_handle_t handle;
    uint8_t modo = POTENCIOMETRO;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) == ESP_OK) {
        nvs_get_u8(handle, NVS_KEY, &modo);
        nvs_close(handle);
    }
    return (selector)(modo <= REGISTROS ? modo : POTENCIOMETRO);
}

void operation_mode_init() {
    nvs_flash_init();
    modo_actual = cargar_modo_nvs();
}

void operation_mode_set(selector nuevo_modo) {
    if (nuevo_modo <= REGISTROS && nuevo_modo != modo_actual) {
        modo_actual = nuevo_modo;
        guardar_modo_nvs(nuevo_modo);
        ESP_LOGI(TAG, "Modo cambiado a %d", nuevo_modo);
    }
}

void operation_mode_task(void *arg){
    selector modo_anterior = -1;

    while (1) {
        if (modo_actual != modo_anterior) {
            if (tarea_actual_handle != NULL) {
                vTaskDelete(tarea_actual_handle);
                tarea_actual_handle = NULL;
            }

            switch (modo_actual) {
                case POTENCIOMETRO:
                    xTaskCreate(lectura_potenciometro, "Leer_pot", 4096, NULL, 1, &tarea_actual_handle);
                    break;

                case TEMPERATURA_NTC:
                    xTaskCreate(tarea_evaluar_temp, "Manejo_por_umbral", 4096, NULL, 2, &tarea_actual_handle);
                    break;

                case REGISTROS:
                    break;
            }

            modo_anterior = modo_actual;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

selector operation_mode_get(void) {
    return modo_actual;
}
