#ifndef MOTOR_H
#define MOTOR_H

/**
 * @brief Inicializa el módulo del motor (TB6612FNG).
 * Configura los pines de dirección (IN1, IN2, STBY) como salidas.
 * Configura el canal PWM (LEDC del ESP32) para el control de velocidad (pin PWM).
 */
void motor_init();

/**
 * @brief Controla la velocidad y dirección del motor.
 * Esta es la función principal para tus pruebas de recolección de datos.
 *
 * @param porcentaje Un valor de -100.0 a +100.0:
 * - +100.0 = Máxima velocidad en sentido horario (adelante).
 * - -100.0 = Máxima velocidad en sentido antihorario (reversa).
 * - 0.0    = Parar (freno).
 */
void motor_move(float porcentaje);

/**
 * @brief Detiene el motor inmediatamente (freno).
 * Es un atajo para motor_move(0).
 */
void motor_stop();

#endif // MOTOR_H