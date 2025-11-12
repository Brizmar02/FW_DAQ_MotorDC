#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

/**
 * @brief Inicializa el módulo del potenciómetro.
 * * Configura el pin del ADC (Conversor Analógico-Digital),
 * la resolución (ej. 12 bits) y la atenuación (rango de voltaje).
 */
void pot_init();

/**
 * @brief Lee el valor analógico crudo del potenciómetro.
 * * Es útil para depuración.
 * * @return int El valor crudo del ADC (ej. 0-4095).
 */
int pot_get_raw_value();

/**
 * @brief Devuelve el valor promedio y filtrado del ADC.
 * Este es el valor que usará el control para la lógica de punto muerto.
 * @return float El valor filtrado (0.0 a 4095.0).
 */
float pot_get_filtered_value();

/**
 * @brief Lee el potenciómetro, filtra el ruido y lo mapea a un valor de RPM.
 * * Esta es la función principal que usará el controlador PID 
 * para saber cuál es el "setpoint" o la velocidad deseada.
 * * @return float El valor de RPM deseado (ej. 0.0 a MAX_RPM).
 */
float pot_get_target_rpm();

#endif // POTENCIOMETRO_H