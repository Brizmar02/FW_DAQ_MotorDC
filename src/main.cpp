/*
 * src/main.cpp
 * Punto de entrada del programa.
 * Simplemente delega todo al módulo de control.
 */

#include <Arduino.h>
#include <control.h> // Incluimos nuestro nuevo módulo "cerebro"

void setup() {
    // Inicializa todo el sistema
    control_init();
}

void loop() {
    // Ejecuta el ciclo de control principal
    control_loop();
}