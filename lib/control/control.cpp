#include "control.h"
#include <config.h>
#include <Arduino.h>

#include <potenciometro.h>
#include <motor.h>
#include <encoder.h>

// Definición del ADC_MAX_RAW (podríamos moverlo a config.h)
#define ADC_MAX_RAW 4095.0

// --- control_init() se queda exactamente igual ---
void control_init() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- INICIANDO CONTROLADOR PRINCIPAL (Modo Bidireccional) ---");
    pot_init();
    motor_init();
    encoder_init(); 
    Serial.println("Sistema listo. Mueve el potenciómetro.");
    }

    /**
     * @brief Función helper para mapear un float (como la de Arduino pero con floats)
     */
    static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// --- ¡LÓGICA DE CONTROL ACTUALIZADA! ---
void control_loop() {
    // 1. Leer el valor filtrado del potenciómetro (0.0 a 4095.0)
    float adc_value = pot_get_filtered_value();

    float porcentaje_motor = 0.0; // Por defecto, el motor está parado

    // 2. Comprobar si estamos fuera de la "zona muerta"
    if (adc_value > (POT_MID_POINT + POT_DEADZONE)) {
        // --- Sección ADELANTE (Horario) ---
        // Mapeamos el rango [Centro+ZonaMuerta, MaxADC] a [0.0, 100.0]
        porcentaje_motor = mapf(adc_value, 
                                POT_MID_POINT + POT_DEADZONE, 
                                ADC_MAX_RAW, 
                                0.0, 
                                100.0);

    } else if (adc_value < (POT_MID_POINT - POT_DEADZONE)) {
        // --- Sección REVERSA (Antihorario) ---
        // Mapeamos el rango [0, Centro-ZonaMuerta] a [-100.0, 0.0]
        porcentaje_motor = mapf(adc_value, 
                                0.0, 
                                POT_MID_POINT - POT_DEADZONE, 
                                -100.0, 
                                0.0);
    }
    // Si estamos DENTRO de la zona muerta, 'porcentaje_motor' se queda en 0.0

    // 3. Limitar el valor por seguridad (evita que mapf se pase de 100)
    if (porcentaje_motor > 100.0) porcentaje_motor = 100.0;
    if (porcentaje_motor < -100.0) porcentaje_motor = -100.0;

    // 4. Mover el motor
    motor_move(porcentaje_motor);

    // 5. Imprimir los datos
    Serial.printf("ADC: %.0f | Porcentaje Motor: %.2f %%\n", adc_value, porcentaje_motor);

    delay(50); 
}