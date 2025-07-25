#ifndef THRESHOLD_MANAGER_H
#define THRESHOLD_MANAGER_H

#include "esp_err.h"

esp_err_t threshold_set(float valor);
esp_err_t threshold_get(float *valor);
esp_err_t threshold_init_with_default(float *valor, float valor_defecto);  // <- NUEVA FUNCIÓN


#endif // THRESHOLD_MANAGER_H
