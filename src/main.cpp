/*
 * src/main.cpp
 * SISTEMA DE RECOLECCIÓN DE DATOS (Versión Final)
 * * Este archivo es solo el punto de entrada. 
 * Toda la lógica reside en el módulo 'lib/control'.
 */

#include <Arduino.h>
#include <control.h> // Importamos el "cerebro" del sistema

void setup() {
    // Inicializa todo el sistema (Sensores, Motor, Encoder, Serial)
    // y realiza las calibraciones necesarias.
    control_init();
}

void loop() {
    // Ejecuta el ciclo principal:
    // 1. Lee Potenciómetro, Encoder y Corriente
    // 2. Controla el Motor
    // 3. Envía datos por Serial (CSV)
    control_loop();
}