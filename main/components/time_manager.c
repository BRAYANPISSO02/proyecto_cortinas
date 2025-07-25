#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "TIME_MANAGER";

void time_manager_init(void)
{
    ESP_LOGI(TAG, "Inicializando SNTP...");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");  // Servidor NTP global
    sntp_init();

    // Esperar a que SNTP sincronice la hora
    time_t now = 0;
    struct tm timeinfo = { 0 };

    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Esperando sincronización de hora (%d/%d)...", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW(TAG, "No se pudo sincronizar la hora con SNTP.");
        return;
    }

    // Establecer zona horaria de Colombia (UTC-5 sin horario de verano)
    setenv("TZ", "America/Bogota", 1);
    tzset();

    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Hora local actual: %s", strftime_buf);
}

esp_err_t time_manager_get_time_string(char *buffer, size_t max_len) {
    time_t now;
    struct tm timeinfo;
    time(&now);

    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2020 - 1900)) {
        return ESP_FAIL;
    }

    strftime(buffer, max_len, "%H:%M:%S", &timeinfo);
    return ESP_OK;
}
