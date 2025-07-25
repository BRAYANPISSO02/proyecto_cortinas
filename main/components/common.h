#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
// Manejo de colas, semáforos y tareas
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
// Calibración GPIOs
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "driver/gpio.h"


// ==========================================
// Definiciones SERVO
// ==========================================
#define SERVO_PIN               GPIO_NUM_38
#define SERVO_CHANNEL           LEDC_CHANNEL_0
#define SERVO_QUEUE_SIZE        1
#define SERVO_MIN_WIDTH_US     500   // Valor mínimo en microsegundos
#define SERVO_MAX_WIDTH_US     2500  // Valor máximo en microsegundos
#define SERVO_MAX_ANGLE        180   // No poner más que el límite físico

// ==========================================
// Configuración de colas y manejadores
// ==========================================

extern QueueHandle_t cola_porcentaje;
extern QueueHandle_t cola_angulo;
extern QueueHandle_t cola_temperatura;
extern QueueHandle_t cola_datos_sensores;

extern adc_oneshot_unit_handle_t adc_handle;
extern adc_cali_handle_t cali_handle_pot;
extern adc_cali_handle_t cali_handle_ntc;

// ==========================================
// Estructuras de datos
// ==========================================

typedef enum {
    POTENCIOMETRO,
    TEMPERATURA_NTC,
    REGISTROS,
} selector;

typedef struct {
    float temperatura_c;
    uint8_t porcentaje;
    uint16_t angulo_servo;
} sensores_data_t;


typedef struct {
    char ssid[32];
    char password[64];
} wifi_credentials_t;

// ==========================================
// REGISTROS
// ==========================================
#define MAX_REGISTROS           5
#define LONGITUD_REGISTRO       16

// ==========================================
// Sensores y ADC
// ==========================================

// Resolución y atenuación ADC
#define ADC_UNIT          ADC_UNIT_1
#define ADC_RESOLUTION    ADC_BITWIDTH_12
#define ADC_ATTEN         ADC_ATTEN_DB_12

// Canal ADC para el potenciómetro
#define POT_CHANNEL       ADC_CHANNEL_0
#define MAX_VOLTAJE_MV    3204

// Canal ADC para el termistor NTC
#define NTC_CHANNEL       ADC_CHANNEL_1
#define NTC_VCC_GPIO      GPIO_NUM_21
#define NTC_VCC           3300.0
#define BETA_NTC          3470
#define R_REF             10000.0
#define R_MALLA           10000.0
#define KELVIN_OFFSET     273.15
#define T0_KELVIN         (25.0 + KELVIN_OFFSET)

// Umbral por defecto
#define DEFAULT_THRESHOLD 25.0

// ==========================================
// WiFi
// ==========================================
#define WIFI_AP_SSID       "ESP32_Control"
#define WIFI_AP_PASSWORD   "control123"
#define WIFI_AP_CHANNEL    6
#define WIFI_AP_MAX_CONN   5
#define WIFI_AP_IP         "192.168.4.1"
#define WIFI_AP_NETMASK    "255.255.255.0"
#define WIFI_AP_GATEWAY    "192.168.4.1"
#define MAX_SSID_LEN        32
#define MAX_PWD_LEN         64

// ==========================================
// Tags para Logging
// ==========================================

#define TAG_MAIN    "Main"
#define TAG_WIFI    "WiFi"
#define TAG_HTTP    "HTTP_SERVER"
#define TAG_RGB     "RGB"
#define TAG_SENSOR  "Sensor"
#define TAG_TIME    "Time"
#define TAG_NVS     "NVS"

#endif // COMMON_H