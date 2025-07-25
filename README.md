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


