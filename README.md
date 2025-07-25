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


