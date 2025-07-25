#ifndef OPERATION_MODE_H
#define OPERATION_MODE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common.h"

void operation_mode_init(void);
void operation_mode_set(selector new_mode);
void operation_mode_task(void *arg);
selector operation_mode_get(void); 
#endif
