**BRAYAN RICARDO PISSO RAMÍREZ**
---

---
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
