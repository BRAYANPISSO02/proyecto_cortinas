#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "esp_err.h"

void time_manager_init(void);
esp_err_t time_manager_get_time_string(char *buffer, size_t max_len);

#endif // TIME_MANAGER_H
