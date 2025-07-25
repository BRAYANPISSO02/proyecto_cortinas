#ifndef REGISTRO_HORARIO_H
#define REGISTRO_HORARIO_H

#include <stdio.h>
#include <stdbool.h>
#include "common.h"

void registro_horario_init(void);
bool registro_horario_guardar(const char *registro_str);
int registro_horario_listar(char registros[][LONGITUD_REGISTRO], size_t max_registros);
void registro_horario_verificar_y_enviar(void);

#endif // REGISTRO_HORARIO_H