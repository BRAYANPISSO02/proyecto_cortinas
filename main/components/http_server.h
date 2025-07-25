#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"

// Inicializa el servidor HTTP y registra todos los URI handlers
void http_server_start(void);

// Detiene el servidor (si alguna vez lo necesitas)
void http_server_stop(void);

#endif // HTTP_SERVER_H
