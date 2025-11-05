#pragma once

/**
 * @file sensores.h
 * @brief Módulo de adquisición para sensores analógicos.
 * * Maneja la lectura del Potenciómetro, Sensor de Corriente (ACS712)
 * * y el Divisor de Voltaje.
 * * Incluye la calibración de offset para el ACS712.
 */

// --- FUNCIONES PÚBLICAS ---

/**
 * @brief Inicializa los pines de los sensores y calibra el offset del ACS712.
 * ¡Importante! Llamar en setup() ANTES de encender el motor.
 */
void sensores_init();

/**
 * @brief Lee el valor crudo del potenciómetro (0-4095).
 * @return El valor ADC del potenciómetro.
 */
int sensores_leer_potenciometro_raw();

/**
 * @brief Mide, filtra y devuelve la corriente del motor en Amperios (A).
 * @return Corriente filtrada en Amperios.
 */
float sensores_leer_corriente();

/**
 * @brief Mide y devuelve el voltaje del motor en Voltios (V).
 * * Respeta el interruptor 'SENSOR_VOLTAJE_HABILITADO' en config.h.
 * @return Voltaje medido en Voltios, o 0.0 si está deshabilitado.
 */
float sensores_leer_voltaje();