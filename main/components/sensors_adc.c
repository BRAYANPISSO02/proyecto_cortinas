#include <stdio.h>
// Manejo de colas y tareas
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// Calibración GPIOs
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "driver/gpio.h"
// Otras necesarias
#include <math.h>
#include "esp_log.h"
#include "common.h"
#include "threshold_manager.h"

// ==========================================
// Configuración de manejadores
// ==========================================

adc_oneshot_unit_handle_t adc_handle = NULL;
adc_cali_handle_t cali_handle_pot = NULL;
adc_cali_handle_t cali_handle_ntc = NULL;

// ==== CONFIGURACIÓN DE ADC ==== //
static adc_oneshot_chan_cfg_t config_adc = {
    .bitwidth = ADC_RESOLUTION,
    .atten = ADC_ATTEN,
};

void iniciar_adc(void) {
    adc_oneshot_unit_init_cfg_t cfg = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&cfg, &adc_handle);
}

adc_cali_handle_t crear_calibracion(adc_channel_t canal, bool *ok_flag) {
    adc_cali_handle_t handle;
    adc_cali_curve_fitting_config_t cfg = {
        .unit_id = ADC_UNIT,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_RESOLUTION,
        .chan = canal
    };
    if (adc_cali_create_scheme_curve_fitting(&cfg, &handle) == ESP_OK) {
        *ok_flag = true;
        return handle;
    } else {
        *ok_flag = false;
        return NULL;
    }
}

// ==========================================
// Lectura del potenciómetro
// ==========================================
void lectura_potenciometro(void *arg) {
    adc_oneshot_config_channel(adc_handle, POT_CHANNEL, &config_adc);
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t periodo = pdMS_TO_TICKS(50);
    uint8_t porcentaje;

    while (1) {
        int raw, mv;
        if (adc_oneshot_read(adc_handle, POT_CHANNEL, &raw) == ESP_OK &&
            cali_handle_pot &&
            adc_cali_raw_to_voltage(cali_handle_pot, raw, &mv) == ESP_OK) {

            porcentaje = mv * 100 / MAX_VOLTAJE_MV;
            xQueueOverwrite(cola_porcentaje, &porcentaje);
        }
        vTaskDelayUntil(&last_wake, periodo);
    }
}   

// ==========================================
// Lectura del termistor NTC
// ==========================================
void configurar_gpio_ntc(void) {
    gpio_config_t conf = {
        .pin_bit_mask = (1ULL << NTC_VCC_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&conf);
    gpio_set_level(NTC_VCC_GPIO, 0);
}

void temperatura_NTC(void *arg) {
    adc_oneshot_config_channel(adc_handle, NTC_CHANNEL, &config_adc);
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t periodo = pdMS_TO_TICKS(100);
    float temperatura;

    while (1) {
        int raw, mv;
        gpio_set_level(NTC_VCC_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(10));

        if (adc_oneshot_read(adc_handle, NTC_CHANNEL, &raw) == ESP_OK &&
            cali_handle_ntc &&
            adc_cali_raw_to_voltage(cali_handle_ntc, raw, &mv) == ESP_OK) {

            float R_ntc = (mv * R_MALLA) / (NTC_VCC - mv);
            float temp_k = 1.0 / (1.0 / T0_KELVIN + log(R_ntc / R_REF) / BETA_NTC);
            temperatura = temp_k - KELVIN_OFFSET;

            xQueueOverwrite(cola_temperatura, &temperatura);
        }

        gpio_set_level(NTC_VCC_GPIO, 0);
        vTaskDelayUntil(&last_wake, periodo);
    }
}

esp_err_t verificar_temperatura_y_activar(void) {
    float temperatura = 0.0;
    float umbral = DEFAULT_THRESHOLD;

    // Leer la temperatura de la cola
    if (xQueuePeek(cola_temperatura, &temperatura, pdMS_TO_TICKS(10)) != pdTRUE) {
        ESP_LOGW("TEMP_CHECK", "No se pudo leer la temperatura");
        return ESP_FAIL;
    }

    // Obtener el umbral desde NVS
    if (threshold_get(&umbral) != ESP_OK) {
        ESP_LOGW("TEMP_CHECK", "No se pudo obtener el umbral desde NVS");
        return ESP_FAIL;
    }

    // Comparar temperatura con umbral
    if (temperatura >= umbral) {
        uint8_t porcentaje = 100;
        if (xQueueOverwrite(cola_porcentaje, &porcentaje) != pdTRUE) {
            ESP_LOGW("TEMP_CHECK", "No se pudo enviar %d%% a la cola de porcentaje", porcentaje);
            return ESP_FAIL;
        }
        ESP_LOGI("TEMP_CHECK", "Temperatura %.2f supera umbral %.2f → Se envió 100%%", temperatura, umbral);
    } else {
        uint8_t porcentaje = 0;
        if (xQueueOverwrite(cola_porcentaje, &porcentaje) != pdTRUE) {
            ESP_LOGW("TEMP_CHECK", "No se pudo enviar %d%% a la cola de porcentaje", porcentaje);
            return ESP_FAIL;
        }
        ESP_LOGI("TEMP_CHECK", "Temperatura %.2f por debajo del umbral %.2f", temperatura, umbral);
    }

    return ESP_OK;
}

void tarea_evaluar_temp(void *pvParameters) {
    while (1) {
        verificar_temperatura_y_activar();
        vTaskDelay(pdMS_TO_TICKS(500));  // Cada 500 ms
    }
}

// ==========================================
// Calibración de los ADC
// ==========================================
void adc_pot_start(void){
    bool ok_pot = false;
    cali_handle_pot = crear_calibracion(POT_CHANNEL, &ok_pot);
}

void adc_ntc_start(void){
    bool ok_ntc = false;
    cali_handle_ntc = crear_calibracion(NTC_CHANNEL, &ok_ntc);
}
