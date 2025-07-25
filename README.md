## ELABORADO POR

 ➤   **Brayan Ricardo Pisso Ramírez CC. 1004249850** 

---

### Fecha de entrega

Viernes, 25 de Julio de 2025 

---

### Materia

> Sistemas En Tiempo Real – Documentación proyecto final  
> Universidad Nacional De Colombia - Sede Manizales

---

# Sistema de Control Inteligente de Cortinas Automatizadas con ESP32, Servomotor y Web Interface

> Proyecto modular basado en ESP32 que permite controlar cortinas mediante tres modos de operación: manual por potenciómetro, automático por umbral de temperatura (NTC) y apertura programada por horario. Incluye una interfaz web para configuración remota y actualización de credenciales WiFi.

## Objetivo General

Diseñar e implementar un sistema de automatización de cortinas basado en el microcontrolador ESP32, que permita su control inteligente mediante tres modos de operación (manual, por temperatura y por horario), integrando sensores analógicos, un servomotor y una interfaz web responsiva para la visualización de datos, selección de modos y configuración remota de parámetros y conectividad.

## Objetivos Específicos

- Controlar la apertura de una cortina utilizando un servomotor operado desde el ESP32 mediante señal PWM.
- Habilitar tres modos de operación seleccionables:
  - Manual: ajuste directo del ángulo de apertura mediante un potenciómetro.
  - Automático: apertura o cierre en función de un umbral de temperatura configurado, medido con un sensor NTC.
  - Programado: control de apertura con base en horarios definidos previamente.
- Desarrollar una interfaz web responsiva alojada en el ESP32 para:
  - Seleccionar el modo de operación.
  - Visualizar la temperatura ambiente y el porcentaje de apertura actual.
  - Configurar el umbral de temperatura y las credenciales WiFi.
- Almacenar de forma persistente los parámetros de configuración (modo, umbral y red WiFi) utilizando la memoria NVS del ESP32.

## Arquitectura del Sistema

El sistema se basa en una arquitectura modular y embebida que integra múltiples componentes de hardware y software para el control automatizado de una cortina mediante un servomotor. La lógica central es ejecutada por un microcontrolador ESP32, que coordina la lectura de sensores, el control del actuador y la comunicación con una interfaz web alojada en su sistema de archivos SPIFFS.

La arquitectura incluye tres capas funcionales principales:

- **Capa de adquisición de datos**: conformada por un potenciómetro y un sensor NTC conectados a los pines analógicos del ESP32. Se utilizan para el modo manual y el modo automático por temperatura, respectivamente.
- **Capa de control y lógica**: implementada en FreeRTOS, permite cambiar dinámicamente entre modos de operación, ejecutar tareas concurrentes, y almacenar configuraciones en memoria no volátil (NVS).
- **Capa de interfaz y comunicación**: proporciona acceso web para visualización de datos y configuración, y gestiona la conectividad WiFi en modo estación o punto de acceso.

### Componentes del sistema

| Componente           | Función principal                                                         |
|----------------------|---------------------------------------------------------------------------|
| **ESP32**            | Unidad de procesamiento central, ejecuta toda la lógica del sistema       |
| **Servomotor**       | Actuador mecánico que ajusta la apertura de la cortina                    |
| **Potenciómetro**    | Entrada analógica para control manual del ángulo del servomotor           |
| **Sensor NTC**       | Entrada analógica para medir la temperatura ambiente                      |
| **SPIFFS**           | Almacenamiento de la interfaz web (HTML, JS, CSS)                         |
| **NVS**              | Almacenamiento persistente de configuración (modo, umbral, WiFi)          |
| **Interfaz Web**     | Plataforma para visualización y control del sistema                       |
| **WiFi (AP/STA)**    | Conexión del ESP32 al cliente para comunicación e interacción remota      |
| **SNTP**             | Sincronización de hora para el modo de operación por horario              |

---

> El sistema está diseñado para ser completamente autónomo y no requiere conexión a servidores externos para su funcionamiento básico, garantizando robustez y flexibilidad en diferentes entornos.

---

## Modos de operación

El sistema cuenta con tres modos de operación que determinan cómo se controla el ángulo de apertura de la cortina. Estos modos son mutuamente excluyentes y pueden ser seleccionados directamente desde la interfaz web. El cambio de modo se refleja en la ejecución dinámica de tareas en el microcontrolador, gracias a la arquitectura basada en FreeRTOS.

### 1. Modo Manual (Potenciómetro)

En este modo, el usuario controla directamente el grado de apertura de la cortina girando un potenciómetro conectado a una entrada analógica del ESP32. El valor leído se interpreta como un porcentaje y se traduce en un ángulo de control PWM que posiciona el servomotor en consecuencia. Es ideal para un control inmediato e intuitivo.

### 2. Modo Automático por Temperatura (Sensor NTC)

Este modo permite automatizar la apertura o cierre de la cortina en función de la temperatura ambiente. El sistema mide continuamente la temperatura mediante un sensor NTC conectado al ESP32 y la compara con un umbral configurable desde la interfaz web. Si la temperatura supera dicho umbral, la cortina se abre o se cierra automáticamente para regular las condiciones del entorno.

> El umbral de activación se guarda en la memoria NVS, lo que permite conservar la configuración incluso después de un reinicio.

### 3. Modo Programado por Horario

En este modo, el sistema opera la cortina con base en horarios definidos por el usuario. Internamente, el ESP32 sincroniza su reloj mediante el protocolo NTP para mantener la hora actualizada. A partir de una tabla de registros horarios almacenada en memoria, el sistema evalúa continuamente si es necesario abrir o cerrar la cortina según la hora del día.

> Este modo es especialmente útil para rutinas diarias como apertura automática en la mañana y cierre por la noche, sin intervención manual.

---

El sistema permite cambiar entre modos sin necesidad de reiniciar el dispositivo, lo que proporciona flexibilidad al usuario final y un comportamiento dinámico adaptable a diferentes necesidades.

## Interfaz Web

La interfaz web es un componente fundamental del sistema, diseñada para ofrecer al usuario una experiencia intuitiva, accesible y sin necesidad de herramientas externas. Está desarrollada utilizando HTML, CSS y JavaScript, y es servida directamente desde el sistema de archivos SPIFFS del ESP32. Esto permite que cualquier dispositivo conectado a la red del ESP32 (ya sea como punto de acceso o cliente WiFi) pueda acceder al panel de control desde un navegador web.

### Funcionalidades principales

La interfaz web permite al usuario:

- **Seleccionar el modo de operación** del sistema (Manual, Automático por temperatura, o Programado por horario).
- **Visualizar en tiempo real** la temperatura medida por el sensor NTC.
- **Consultar el porcentaje de apertura actual** del servomotor.
- **Configurar el umbral de temperatura** para el modo automático.
- **Actualizar las credenciales de red WiFi**, permitiendo la conexión del ESP32 a una red local sin necesidad de reprogramar el firmware.

### Estructura de la interfaz

La interfaz está organizada en secciones bien diferenciadas, cada una con campos de entrada y botones de acción. Internamente, el archivo `script.js` se encarga de realizar solicitudes `fetch()` a rutas del servidor embebido (como `/api/get_temperatura`, `/api/set_threshold.json`, etc.), permitiendo una comunicación asincrónica y eficiente entre el navegador y el ESP32.

### Seguridad y almacenamiento

- La configuración enviada desde la interfaz es almacenada en la memoria no volátil (NVS), asegurando su persistencia tras reinicios.
- El sistema incluye validaciones básicas de los campos ingresados por el usuario para prevenir errores durante el envío de configuraciones.

### Compatibilidad

La interfaz es completamente responsiva, lo que significa que se adapta correctamente a distintos tamaños de pantalla (PC, tablets, smartphones), permitiendo controlar el sistema cómodamente desde cualquier dispositivo.

---

Gracias a esta interfaz, el sistema puede ser configurado, monitoreado y operado de forma remota y sencilla, sin requerir aplicaciones móviles ni intervención física directa en el microcontrolador.

## Diagrama de flujo del funcionamiento
<img width="772" height="1071" alt="Funcionamiento proyecto RTOS" src="https://github.com/user-attachments/assets/c46cdc27-e5cf-48a7-81e9-bdc14f7fc741" />

## Explicación del flujo programa

El código del proyecto está estructurado de forma modular, dividiendo responsabilidades en distintos archivos fuente, y usando FreeRTOS para gestionar tareas concurrentes. A continuación, se describe el flujo general de ejecución y la función de cada parte principal del sistema.

### 1. Inicialización del sistema (`main.c`)

- En el arranque, se inicializan:
  - NVS (almacenamiento no volátil)
  - SPIFFS (sistema de archivos para la interfaz web)
  - Conexión WiFi (modo STA o AP según estado)
  - Servidor HTTP embebido
  - Colas para comunicación entre tareas (`QueueHandle_t`)
- Se determina el **modo de operación activo** leyendo desde NVS (`operation_mode.c`).
- Se lanza una tarea principal (`servo_queue`) que se encarga de activar dinámicamente la tarea correspondiente al modo seleccionado.

### 2. Manejo de modos (`operation_mode.c`)

- Define un `enum` con los tres modos: `POTENCIOMETRO`, `TEMPERATURA_NTC`, `REGISTROS`.
- Permite obtener y establecer el modo desde el servidor web o desde el propio código.
- El valor del modo se almacena persistentemente en NVS.

### 3. Tareas de control de sensores (`sensors_adc.c`)

- Para el modo manual:
  - Se ejecuta la tarea `lectura_potenciometro`, que convierte la lectura del ADC en un porcentaje (0-100%) y lo envía a la cola del servomotor.
- Para el modo automático:
  - Se ejecuta la tarea `temperatura_NTC`, que lee el sensor NTC, calcula la temperatura real y la compara con el umbral configurado. Según el resultado, se ajusta el ángulo del servomotor.

### 4. Control por horario (`registro_horario.c` + `time_manager.c`)

- El módulo `registro_horario.c` administra una tabla de eventos programados para apertura y cierre.
- Se basa en la hora del sistema, que se sincroniza vía NTP (`time_manager.c`) al inicio.
- Evalúa periódicamente si debe cambiar el estado del servomotor con base en la hora actual.

### 5. Control del servomotor (`iot_servo.c`)

- Usa el periférico `LEDC` del ESP32 para generar señales PWM.
- Recibe los valores de ángulo o porcentaje desde las colas y actualiza la posición del servomotor.

### 6. Umbral de temperatura (`threshold_manager.c`)

- Permite guardar y recuperar el valor del umbral desde NVS.
- El valor se puede modificar desde la interfaz web mediante la API `/api/set_threshold.json`.

### 7. Gestión WiFi (`wifi_manager.c`)

- Permite al usuario cambiar las credenciales de WiFi desde la interfaz web.
- El sistema guarda estas credenciales en NVS y reinicia automáticamente el ESP32 para aplicar los cambios.

### 8. Servidor Web (`http_server.c`)

- Monta el sistema SPIFFS y sirve los archivos HTML, JS y CSS.
- Expone endpoints REST (`/api/...`) que permiten:
  - Obtener la temperatura actual.
  - Consultar y modificar el umbral.
  - Cambiar el modo de operación.
  - Consultar el estado del servomotor.
  - Actualizar las credenciales de WiFi.

---

## Requisitos del Proyecto

### Requisitos de Hardware

A continuación se listan los componentes físicos necesarios para implementar el sistema de control de cortinas automatizado:

| Componente             | Descripción                                                        |
|------------------------|--------------------------------------------------------------------|
| ESP32 DevKit (modelo ESP32-WROOM o ESP32-S3) | Microcontrolador principal con conectividad WiFi y soporte FreeRTOS |
| Servomotor SG90 (o similar)       | Actuador para apertura/cierre de la cortina                              |
| Sensor NTC 10k                   | Sensor de temperatura resistivo analógico                                |
| Potenciómetro lineal (10kΩ)      | Dispositivo para control manual de apertura                              |
| Fuente de alimentación 5 V / 2 A | Para alimentar el ESP32 y el servomotor                                  |
| Jumpers macho-macho              | Conexiones entre módulos y la placa ESP32                                |
| Protoboard o PCB                 | Plataforma para conexión segura de componentes                           |
| Cables USB                       | Para conexión del ESP32 al computador y carga del firmware               |

> Nota: En caso de usar un servomotor de torque más alto, se recomienda alimentarlo con una fuente externa independiente del ESP32.

---

### Requisitos de Software

Estos son los programas y entornos necesarios para compilar, cargar y ejecutar el proyecto:

| Herramienta / Entorno       | Uso principal                                                  |
|-----------------------------|-----------------------------------------------------------------|
| [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) | Framework oficial de desarrollo para ESP32                     |
| Python 3.x                  | Dependencia requerida por ESP-IDF para scripts de build y carga |
| Terminal (cmd, bash, etc.)  | Para compilar y flashear el proyecto mediante CLI              |
| Navegador web moderno (Chrome, Firefox, Edge) | Para acceder a la interfaz web del sistema                      |
| Editor de texto / IDE (opcional) | Visual Studio Code, ESP-IDF Plugin, o similar                   |

> Recomendado: utilizar el plugin oficial de ESP-IDF para Visual Studio Code para facilitar la edición, compilación y monitoreo del sistema.

---

## Pruebas y Validación

Con el fin de garantizar el correcto funcionamiento del sistema en sus distintos modos, se realizaron pruebas funcionales específicas para cada uno de ellos. A continuación se describen los procedimientos de prueba, los resultados esperados y las observaciones correspondientes.

### 1. Prueba del modo manual (potenciómetro)

**Procedimiento**:
- Girar el potenciómetro desde 0 % hasta 100 %.
- Observar la respuesta del servomotor.
- Verificar en la interfaz web el valor de apertura en porcentaje.

**Resultado esperado**:
- El servomotor debe moverse suavemente de 0° a 180° (según el rango definido).
- El porcentaje mostrado en la interfaz debe coincidir con la posición del potenciómetro.

**Resultado obtenido**:
- Movimiento continuo y estable. Visualización correcta en tiempo real.

---

### 2. Prueba del modo automático por temperatura

**Procedimiento**:
- Configurar un umbral desde la interfaz web (por ejemplo, 28 °C).
- Aumentar la temperatura sobre el sensor (ej. con calor del cuerpo o secador).
- Observar el comportamiento del servomotor.

**Resultado esperado**:
- Si la temperatura supera el umbral, el sistema debe accionar la apertura/cierre.
- El valor de temperatura debe actualizarse dinámicamente en la interfaz.

**Resultado obtenido**:
- Activación precisa al alcanzar el umbral.
- Visualización correcta de la temperatura.

---

### 3. Prueba del modo programado por horario

**Procedimiento**:
- Definir horarios de apertura y cierre en el módulo `registro_horario.c`.
- Verificar que la hora del sistema esté sincronizada vía NTP.
- Observar el cambio de estado del servomotor en los momentos programados.

**Resultado esperado**:
- El sistema debe ejecutar la acción asignada exactamente a la hora programada.

**Resultado obtenido**:
- Comportamiento correcto tras sincronización NTP. Acciones ejecutadas puntualmente.

---

### 4. Prueba de persistencia de configuración

**Procedimiento**:
- Cambiar modo, umbral y credenciales desde la interfaz web.
- Reiniciar manualmente el ESP32.
- Observar si los valores permanecen cargados al reinicio.

**Resultado esperado**:
- La configuración debe persistir gracias al almacenamiento en NVS.

**Resultado obtenido**:
- Todos los parámetros permanecen almacenados correctamente después del reinicio.

---

### 5. Prueba de cambio de red WiFi

**Procedimiento**:
- Ingresar nuevos datos de red en la sección correspondiente de la interfaz.
- Esperar reinicio automático del sistema.
- Confirmar que el ESP32 se conecte a la nueva red.

**Resultado esperado**:
- Conexión exitosa a la red configurada.

**Resultado obtenido**:
- Sin conexión establecida en modo estación (STA).

---

## Diagrama de estructura del proyecto

```
project/
│
├── main.c
├── components/
│   ├── http_server.c
│   ├── wifi_manager.c
│   ├── iot_servo.c
│   ├── sensors_adc.c
│   ├── operation_mode.c
│   └── ...
└── web_page/
    ├── index.html
    └── script.js
```

