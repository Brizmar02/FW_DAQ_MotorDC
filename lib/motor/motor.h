#pragma once

/**
 * @file motor.h
 * @brief Módulo de control para el driver de motor TB6612FNG.
 * * Encapsula la inicialización del PWM (LEDC) y la lógica de
 * pines (IN1, IN2, STBY) para controlar el motor.
 */

// --- FUNCIONES PÚBLICAS ---

/**
 * @brief Inicializa el periférico LEDC para PWM y configura los pines.
 * Debe llamarse una vez en el setup().
 */
void motor_init();

/**
 * @brief Controla el motor con una velocidad y dirección dadas.
 * * @param duty_cycle El valor de "potencia" (0 a PWM_MAX_DUTY)
 * @param direction La dirección (true = adelante, false = atrás)
 */
void motor_drive(int duty_cycle, bool direction);

/**
 * @brief Detiene el motor aplicando un "freno corto" (ambos pines a LOW).
 * Esto es más rápido que simplemente apagar el STBY.
 */
void motor_brake();