#include <stdio.h>
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "esp_log.h"
// Manejo de colas, semáforos y tareas
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
// Calibración GPIOs
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "driver/gpio.h"
// Componentes del proyecto
#include "components/iot_servo.h"
#include "components/common.h"
#include "components/sensors_adc.h"
#include "components/operation_mode.h"
#include "components/http_server.h"
#include "components/wifi_manager.h"

// Inclusión de los componentes globales
QueueHandle_t cola_porcentaje = NULL;
QueueHandle_t cola_angulo = NULL;
QueueHandle_t cola_temperatura = NULL;
QueueHandle_t cola_datos_sensores = NULL;


// ==========================================
// Operación del SERVO
// ==========================================
/*
selector modo_actual = POTENCIOMETRO;   // Se puede cambiar desde UART, botón, web, etc.

TaskHandle_t tarea_actual_handle = NULL;

void servo_queue(void *arg){
    selector modo_anterior = -1;

    while (1) {
        if (modo_actual != modo_anterior) {
            // Si hay una tarea previa, eliminarla
            if (tarea_actual_handle != NULL) {
                vTaskDelete(tarea_actual_handle);
                tarea_actual_handle = NULL;
            }

            // Crear la nueva tarea según el modo
            switch (modo_actual) {
                case POTENCIOMETRO:
                    adc_pot_start();
                    xTaskCreate(lectura_potenciometro, "Leer_potenciometro", 4096, NULL, 1, &tarea_actual_handle);
                    break;

                case TEMPERATURA_NTC:
                    configurar_gpio_ntc();
                    adc_ntc_start();
                    xTaskCreate(temperatura_NTC, "Leer_temperatura_ntc", 4096, NULL, 1, &tarea_actual_handle);
                    break;

                case REGISTROS:
                    break;
            }

            modo_anterior = modo_actual;
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // Espera para evitar sobrecarga
    }
}
*/

void servo_angle(void *arg) {
    u_int8_t porcentaje;

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t periodo = pdMS_TO_TICKS(50);
    while (1) {
        if (xQueuePeek(cola_porcentaje, &porcentaje, pdMS_TO_TICKS(10)) == pdTRUE) {
            float angulo = (porcentaje * SERVO_MAX_ANGLE) / 100.0f;
            xQueueOverwrite(cola_angulo, &angulo);
        }
        vTaskDelayUntil(&last_wake, periodo);
    }
}


void servo_control(void *arg) {
    float angulo;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t periodo = pdMS_TO_TICKS(50);
    while (1) {
        if (xQueueReceive(cola_angulo, &angulo, pdMS_TO_TICKS(10)) == pdTRUE) {
            iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, angulo);
        }
        vTaskDelayUntil(&last_wake, periodo);
    }
}

// ==========================================
// Principal
// ==========================================
void app_main(void) {

    // ==== Crear colas ====
    cola_porcentaje = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(uint8_t));
    cola_angulo = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(float));
    cola_temperatura = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(float));
    cola_datos_sensores = xQueueCreate(SERVO_QUEUE_SIZE, sizeof(sensores_data_t));

    // ==== Inicializar Servo ====
    servo_config_t servo_cfg = {
        .max_angle = SERVO_MAX_ANGLE,
        .min_width_us = SERVO_MIN_WIDTH_US,
        .max_width_us = SERVO_MAX_WIDTH_US,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin = { SERVO_PIN },
            .ch = { SERVO_CHANNEL },
        },
        .channel_number = 1
    };
    iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);

    // 1. Inicialización básica
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 4. Conectividad
    wifi_manager_init();
    vTaskDelay(pdMS_TO_TICKS(3000));
    wifi_manager_try_connect_saved();


    iniciar_adc();
    adc_ntc_start();
    adc_pot_start();
    operation_mode_init();
    configurar_gpio_ntc();


    http_server_start();

    // ==== Tareas control servo ====
    xTaskCreate(servo_angle, "Porcentaje_a_angulo", 4096, NULL, 2, NULL);
    xTaskCreate(servo_control, "Control_servo", 4096, NULL, 2, NULL);
    // xTaskCreate(change_mode, "Cambio_de_modo_de_operacion", 4096, NULL, 5, NULL);
    // xTaskCreate(servo_queue, "Modo_operacion", 4096, NULL, 3, NULL);
    xTaskCreate(temperatura_NTC, "Leer_ntc", 4096, NULL, 1, NULL);
    xTaskCreate(operation_mode_task, "Modo_operacion_servo", 4096, NULL, 3, NULL);
}

