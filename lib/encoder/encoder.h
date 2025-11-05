#pragma once

#include <Arduino.h>

/**
 * @file encoder.h
 * @brief Módulo para leer el encoder en cuadratura usando el hardware PCNT.
 * * Es no-bloqueante y resistente a ruido/pérdida de pulsos.
 */

// --- FUNCIONES PÚBLICAS ---

/**
 * @brief Inicializa el periférico PCNT (Pulse Counter) del ESP32-S3.
 * Debe llamarse una vez en el setup().
 */
void encoder_init();

/**
 * @brief Resetea el contador de posición actual a cero.
 */
void encoder_reset_position();

/**
 * @brief Obtiene el conteo de pulsos actual (posición relativa).
 * @return El número de pulsos desde el último reseteo.
 */
int64_t encoder_get_position();

/**
 * @brief Calcula y devuelve la velocidad actual del eje.
 * * ¡Importante! Esta función DEBE ser llamada regularmente
 * * en el loop() para que el cálculo sea preciso.
 * @return Velocidad calculada en RPM (Revoluciones Por Minuto).
 */
float encoder_get_velocity_rpm();