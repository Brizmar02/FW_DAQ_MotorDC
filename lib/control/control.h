#ifndef CONTROL_H
#define CONTROL_H

/**
 * @brief Inicializa todos los módulos de hardware
 * (Serial, potenciómetro, motor, encoder).
 */
void control_init();

/**
 * @brief Ejecuta un ciclo de la lógica de control principal.
 * Lee el potenciómetro, calcula el control y mueve el motor.
 * Esta función está diseñada para ser llamada repetidamente en el loop() de main.
 */
void control_loop();

#endif // CONTROL_H