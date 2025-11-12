#ifndef SENSORES_H
#define SENSORES_H

/**
 * @brief Inicializa los sensores de corriente y voltaje.
 * ¡IMPORTANTE! Esta función debe llamarse en setup(), ANTES
 * de que el motor reciba cualquier comando de movimiento.
 * Se usa para calibrar el "punto cero" (offset) del sensor ACS712.
 */
void sensores_init();

/**
 * @brief Lee el sensor de corriente y devuelve el valor filtrado en Amperios (A).
 * Utiliza un cálculo de RMS para obtener una lectura precisa
 * incluso bajo la carga de un motor con PWM.
 *
 * @return float La corriente (RMS) que consume el motor en Amperios.
 */
float sensor_get_corriente_A();

/**
 * @brief Lee el sensor de voltaje (divisor) y devuelve el valor en Voltios (V).
 * Si el sensor está deshabilitado en config.h, devolverá 0.0.
 *
 * @return float El voltaje de la fuente de alimentación del motor (ej. 11.5V).
 */
float sensor_get_voltaje_V();

#endif // SENSORES_H